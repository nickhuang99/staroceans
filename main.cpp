#include <iostream>
#include "MyCapture.h"
#include "MySDLDisplay.h"
#include "MyX264.h"

using namespace std;

static unsigned char yBuf[640*480];
static unsigned char uBuf[640/2*480];
static unsigned char vBuf[640/2*480];

void writePlanarYUV(FILE* output, unsigned char* ptr)
{
	for (unsigned int r = 0; r < 480; r ++)
	{
		fwrite(ptr+r*600*2, 1, 600*2, output);
	}
}

void writePlanarYUV3(FILE* output, unsigned char* ptr)
{
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < 480; r ++)
	{
		if (r % 2 == 0)
		{
			for (unsigned int c = 0; c < 300; c ++)
			{
				*yPtr ++ = *ptr ++;
				*uPtr ++ = *ptr ++;
				*yPtr ++ = *ptr ++;
				*vPtr ++ = *ptr ++;
			}
		}
		else
		{
			for (unsigned int c = 0; c < 600; c ++)
			{
				*yPtr ++ = *ptr ++;
			}
		}

	}
	fwrite(yBuf, 1, 600*480, output);
	fwrite(uBuf, 1, 600*480/4, output);
	fwrite(vBuf, 1, 600*480/4, output);
}


void writePlanarYUV2(FILE* output, unsigned char* ptr)
{
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < 480; r ++)
	{
		if (r % 2 == 0)
		{
			for (unsigned int c = 0; c < 600; c ++)
			{
				*yPtr ++ = *ptr ++;
				*uPtr ++ = *ptr ++;
			}
		}
		else
		{
			for (unsigned int c = 0; c < 600; c ++)
			{
				*yPtr ++ = *ptr ++;
				*vPtr ++ = *ptr ++;
			}
		}
	}
	fwrite(yBuf, 1, 600*480, output);
	fwrite(uBuf, 1, 600*480/4, output);
	fwrite(vBuf, 1, 600*480/4, output);
}
void writePlanarYUV1(FILE* output, unsigned char* ptr)
{
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;

	for (unsigned int r = 0; r < 480; r ++)
	{
		for (unsigned int c = 0; c < 300; c ++)
		{
			*yPtr ++ = *ptr ++;
			*uPtr ++ = *ptr ++;
			*yPtr ++ = *ptr ++;
			*vPtr ++ = *ptr ++;
		}
	}
	fwrite(yBuf, 1, 600*480, output);
	fwrite(uBuf, 1, 600/2*480, output);
	fwrite(vBuf, 1, 600/2*480, output);
}


int main()
{

    MyVideoCapture capture;
    MySDLDisplay display;
    MyX264 x264;
    if (!x264.init(600,480, "output.raw"))
    {
    	printf("unable to initialize x264/n");
    	return -1;
    }
    //FILE* output = fopen("output.raw", "w");
    //capture.startCaptureAndDisplay();

    size_t size = capture.startCapture();
    if (size > 0 && display.init() && display.getImgSize() == size)
    {
        unsigned char* ptr = new unsigned char[size];
        if (ptr)
        {
            int counter = 0;
            struct timeval last, now;
            while (counter <100)
            {
                if (counter % MyVideoCapture::FPS_COUNT_NUMBER == 0 )
                {
                    gettimeofday(&last, NULL);
                }
                if (!capture.captureFrame(ptr, false))
                {
                    break;
                }
                if (!display.displayImage(ptr))
                {
                    break;
                }

                if (!x264.encode_frame(ptr, display.getImgSize()))
                {
                	break;
                }

                //writePlanarYUV(output, ptr);
                counter ++;
                if (counter % MyVideoCapture::FPS_COUNT_NUMBER == 0)
                {
                    gettimeofday(&now, NULL);
                    double diff = (now.tv_sec - last.tv_sec) + (double)(now.tv_usec - last.tv_usec)/1000000.0;

                    printf("fps:%f\n", (double)MyVideoCapture::FPS_COUNT_NUMBER/diff);
                }
            }
            delete [] ptr;
        }
    }
    //fclose(output);
    return 0;
}
