#ifndef MYCAPTURE_H_INCLUDED
#define MYCAPTURE_H_INCLUDED

#include <libv4l2.h>
#include <linux/videodev2.h>
#include <pthread.h>


class MyVideoCapture
{
public:
    void startCaptureAndDisplay(unsigned long pixelFormat = V4L2_PIX_FMT_YUYV,
               unsigned long width = 640, unsigned long height = 480,
               size_t bufferNumber = 2);
    size_t startCapture(unsigned long pixelFormat = V4L2_PIX_FMT_YUYV,
               unsigned long width = 640, unsigned long height = 480,
               size_t bufferNumber = 10);
    bool captureFrame(unsigned char*ptr, bool bSkip = false);
    bool captureFrame(unsigned char*ptr[4], bool bSkip = false);
    ~MyVideoCapture();
    MyVideoCapture();
    static const int FPS_COUNT_NUMBER=30;
protected:
    static int m_fd;
    pthread_t m_pthread;
    char m_dev_name[32];
    unsigned long m_pixelFormat;
    unsigned long m_width, m_height;
    struct v4l2_format m_fmt;
    static size_t m_bufferNumber;
    static void** m_startArray;
    static size_t m_length;

    bool open_device();

    bool init_device();

    bool init_buffer();

    void close_device();
    static void* mainloop(void* arg);
    static int xioctl(int fd, int request, void* arg);
};


#endif // MYCAPTURE_H_INCLUDED
