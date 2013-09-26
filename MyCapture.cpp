#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h>          /* for videodev2.h */
#include <time.h>
#include "MyCapture.h"
#include "MySDLDisplay.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
static FILE* before, *after;
int MyVideoCapture::m_fd = -1;
void** MyVideoCapture::m_startArray = NULL;
size_t MyVideoCapture::m_length = 0;
size_t MyVideoCapture::m_bufferNumber=0;

MyVideoCapture::MyVideoCapture()
{
    m_pixelFormat = V4L2_PIX_FMT_YUYV;
    m_width = 600;
    m_height = 480;
    m_startArray = NULL;
    m_length = 0;
    m_bufferNumber = 0;
}

MyVideoCapture::~MyVideoCapture()
{
    close_device();
}


bool MyVideoCapture::open_device()
{
    for (int i = 0; i < 64; i ++)
    {
        struct stat st;
        sprintf(m_dev_name, "/dev/video%d", i);
        if (0 == stat (m_dev_name, &st))
        {
            if (S_ISCHR (st.st_mode))
            {
                if ((m_fd = v4l2_open(m_dev_name, O_RDWR | O_NONBLOCK, 0)) != -1)
                {
                    if (init_device())
                    {
                        return true;
                    }
                    v4l2_close(m_fd);
                }
            }
        }
    }
    return false;
}

int MyVideoCapture::xioctl(int fd, int request, void* arg)
{
    int r;
    do
    {
        r = ioctl (fd, request, arg);
    }
    while (-1 == r && EINTR == errno);

    return r;
}


bool MyVideoCapture::init_device()
{
    struct v4l2_capability cap;

    if (-1 == xioctl (m_fd, VIDIOC_QUERYCAP, &cap))
    {
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        || !(cap.capabilities & V4L2_CAP_STREAMING))
    {
        return false;
    }
    v4l2_fmtdesc desc;
    desc.index = 0;
    desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (xioctl(m_fd, VIDIOC_ENUM_FMT, &desc) != -1)
    {
        if (desc.pixelformat == m_pixelFormat)
        {
            CLEAR (m_fmt);
            m_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            m_fmt.fmt.pix.pixelformat = m_pixelFormat;

            m_fmt.fmt.pix.width = m_width;
            m_fmt.fmt.pix.height = m_height;
            if (-1 == xioctl (m_fd, VIDIOC_S_FMT, &m_fmt))
            {
                return false;
            }
            else
            {
                return init_buffer();
            }

        }
        desc.index ++;
    }
    return false;
}
bool MyVideoCapture::init_buffer()
{
    struct v4l2_requestbuffers req;
    m_startArray = new void*[m_bufferNumber];
    memset(m_startArray, 0, sizeof(void*)*m_bufferNumber);
    if (m_startArray == NULL)
    {
        return false;
    }
    CLEAR(req);
    req.count = m_bufferNumber;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(m_fd, VIDIOC_REQBUFS, &req) == -1)
    {
        return false;
    }

    struct v4l2_buffer buf;
    CLEAR(buf);
    bool bResult = true;
    for (size_t i = 0; i < m_bufferNumber; i ++)
    {
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;

        if (xioctl(m_fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            bResult = false;
            break;
        }

        m_length = buf.length;
        m_startArray[i] = v4l2_mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd,
                            buf.m.offset);

        if (MAP_FAILED == m_startArray[i])
        {
            bResult = false;
            break;
        }
        if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
        {
            bResult = false;
            break;

        }
    }
    if (bResult)
    {
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(m_fd, VIDIOC_STREAMON, &type) == -1)
        {
            return false;
        }
    }
    return bResult;
}

void MyVideoCapture::close_device()
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(m_fd, VIDIOC_STREAMOFF, &type);
    if (m_startArray != NULL)
    {
        for (size_t i = 0; i < m_bufferNumber; i ++)
        {
            v4l2_munmap(m_startArray[i], m_length);
        }
        delete []m_startArray;
        m_startArray = NULL;
        m_length = 0;
    }
    if (m_fd != -1)
    {
        v4l2_close(m_fd);
        m_fd = -1;
        fclose(before);
        fclose(after);
    }
}

void MyVideoCapture::startCaptureAndDisplay(unsigned long pixelFormat,
               unsigned long width, unsigned long height, size_t bufferNumber)
{
    if (m_fd == -1)
    {
        m_pixelFormat = pixelFormat;
        m_width = width;
        m_height = height;
        m_bufferNumber = bufferNumber;
        if (open_device())
        {
            if (pthread_create(&m_pthread, NULL, &MyVideoCapture::mainloop, NULL) == 0)
            {
                pthread_join(m_pthread, NULL);
                close_device();
            }
        }
        else
        {
            close_device();
        }
    }
}

size_t MyVideoCapture::startCapture(unsigned long pixelFormat,
               unsigned long width, unsigned long height, size_t bufferNumber)
{
    size_t size = 0;
    if (m_fd == -1)
    {
        m_pixelFormat = pixelFormat;
        m_width = width;
        m_height = height;
        m_bufferNumber = bufferNumber;
        if (open_device())
        {
            size = m_length;
            before = fopen("before", "w");
            after = fopen("after", "w");
        }
        else
        {
            close_device();

        }
    }
    return size;
}

bool MyVideoCapture::captureFrame(unsigned char*dstPtr, bool bSkip)
{
    int r;
    do
    {
        struct timeval tv;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);
        /* Timeout. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        r = select(m_fd + 1, &fds, NULL, NULL, &tv);
    }
    while ((r == -1 && (errno = EINTR)));
    if (r == -1)
    {
        return false;
    }
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1)
    {
        return false;
    }
    unsigned char* srcPtr = (unsigned char*)m_startArray[buf.index];
    if (!bSkip)
    {
        memcpy(dstPtr, srcPtr, m_length);
    }

    if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
    {
        return false;
    }
    return true;
}

bool MyVideoCapture::captureFrame(unsigned char*dstPtr[4], bool bSkip)
{
    int r;
    do
    {
        struct timeval tv;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);
        /* Timeout. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        r = select(m_fd + 1, &fds, NULL, NULL, &tv);
    }
    while ((r == -1 && (errno = EINTR)));
    if (r == -1)
    {
        return false;
    }
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1)
    {
        return false;
    }
    if (!bSkip)
    {
        unsigned char* srcPtr = (unsigned char*)m_startArray[buf.index];

        unsigned char* yPtr = dstPtr[0], *uPtr=dstPtr[1], *vPtr= dstPtr[2];

        //fwrite(srcPtr, m_length, 1, before);
        // convert from packed yuv422 to planar yuv420
        bool bEvenRow = true;
        for (size_t r = 0; r < m_height; r ++)
        {
            if (bEvenRow)
            {
                for (size_t c = 0; c < m_width/2; c ++)
                {
                    *yPtr++ = *srcPtr++;
                    *uPtr++ = *srcPtr++;
                    *yPtr++ = *srcPtr++;
                    *vPtr++ = *srcPtr++;
                }
            }
            else
            {
                for (size_t c = 0; c < m_width/2; c ++)
                {
                    *yPtr++ = *srcPtr++;
                    srcPtr++;
                    *yPtr++ = *srcPtr++;
                    srcPtr++;
                }

            }
            //bEvenRow = !bEvenRow;
        }
        //fwrite(dstPtr[0], m_width*m_height, 1, after);
        //fwrite(dstPtr[1], m_width*m_height/4, 1, after);
        //fwrite(dstPtr[2], m_width*m_height/4, 1, after);
    }

    if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
    {
        return false;
    }
    return true;
}


void* MyVideoCapture::mainloop(void* arg)
{
    MySDLDisplay display;
    SDL_Event event;
    struct timeval last, now;
    int counter = 0;
    if (!display.init())
    {
        return NULL;
    }
    while (true)
    {
        if (counter == 0 )
        {
            gettimeofday(&last, NULL);
        }

        SDL_PollEvent(&event);
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
        {
            break;
        }
        int r;
        do
        {
            struct timeval tv;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(m_fd, &fds);
            /* Timeout. */
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            r = select(m_fd + 1, &fds, NULL, NULL, &tv);
        }
        while ((r == -1 && (errno = EINTR)));
        if (r == -1)
        {
            perror("select");
            break;
        }
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1)
        {
            break;
        }

        if (!display.displayImage((unsigned char*)m_startArray[buf.index]))
        {
            break;
        }

        if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
        {
            break;
        }
        counter ++;
        if (counter % FPS_COUNT_NUMBER == 0)
        {
            gettimeofday(&now, NULL);
            double diff = (now.tv_sec - last.tv_sec) + (double)(now.tv_usec - last.tv_usec)/1000000.0;
            counter = 0;
            printf("fps:%f\n", (double)FPS_COUNT_NUMBER/diff);

        }
    }

    return NULL;
}

/*
int main()
{
    MyVideoCapture* ptr = MyVideoCapture::createInstance();
    void* pStart = NULL;
    size_t length = 0;
    if (ptr->start(&pStart, &length))
    {
        ptr->end();
    }
    return 0;

}


bool MyVideoCapture::captureFrame(unsigned char*dstPtr[4], bool bSkip)
{
    int r;
    do
    {
        struct timeval tv;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        r = select(m_fd + 1, &fds, NULL, NULL, &tv);
    }
    while ((r == -1 && (errno = EINTR)));
    if (r == -1)
    {
        return false;
    }
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1)
    {
        return false;
    }
    unsigned char* srcPtr = (unsigned char*)m_startArray[buf.index];
    if (!bSkip)
    {
        size_t planeSize = m_width*m_height;
        unsigned char* yPtr = dstPtr[0], *uPtr=dstPtr[1], *vPtr= dstPtr[2];
        bool bEvenRow = true;
        for (size_t r = 0; r < m_height; r ++)
        {
            bool bEvenCol = true;
            for (size_t c = 0; c < m_width; c ++)
            {
                *yPtr = *srcPtr++;
                if (bEvenRow)
                {
                    if (bEvenCol)
                    {
                        bEvenCol = false;
                        *uPtr++ = *srcPtr++;
                        *vPtr++ = *srcPtr++;
                    }
                    else
                    {
                        bEvenCol = true;
                    }
                }
                else
                {
                    if (bEvenCol)
                    {
                        bEvenCol = false;
                        srcPtr++;
                        srcPtr++;
                    }
                    else
                    {
                        bEvenCol = true;
                    }
                }
            }
            if (bEvenRow)
            {
                bEvenRow = false;
            }
            else
            {
                bEvenRow = true;
            }
        }
    }

    if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
    {
        return false;
    }
    return true;
}
*/
