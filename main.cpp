#include <iostream>
#include "MyCapture.h"
#include "MySDLDisplay.h"
#include "MyX264.h"

using namespace std;

const unsigned long Width = 640;
const unsigned long Height = 480;
static unsigned char yBuf[Width*Height];
static unsigned char uBuf[Width*Height];
static unsigned char vBuf[Width*Height];

void writePlanarYUV3(FILE* output, unsigned char* ptr)
{
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < Height; r ++)
	{
		for (unsigned int c = 0; c < Width/2; c ++)
		{
			*yPtr = *ptr;
			yPtr ++;
			ptr ++;
			*uPtr = *ptr;
			uPtr ++;
			ptr ++;
			*yPtr = *ptr;
			yPtr ++;
			ptr ++;
			*vPtr = *ptr;
			vPtr ++;
			ptr ++;
		}
	}
	fwrite(yBuf, 1, Width*Height, output);
	fwrite(uBuf, 1, Width*Height/2, output);
	fwrite(vBuf, 1, Width*Height/2, output);
}

// i420
void writePlanarYUV(FILE* output, unsigned char* ptr)
{
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < Height; r ++)
	{
		if (r % 2 == 0)
		{
			for (unsigned int c = 0; c < Width; c ++)
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
	fwrite(yBuf, 1, Width*Height, output);
	fwrite(uBuf, 1, Width*Height/4, output);
	fwrite(vBuf, 1, Width*Height/4, output);
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
	static FILE* output = NULL;
	if (output == NULL)
	{
		output = fopen("output.yuv", "w+b");
	}
    MyVideoCapture capture;
    MySDLDisplay display;
    MyX264 x264;
    if (!x264.init(capture.getImageWidth(), capture.getImageHeight(), "output.raw"))
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
/*
                if (!x264.encode_frame(ptr, display.getImgSize()))
                {
                	break;
                }
*/
                //if (counter == 10)
                {
                	writePlanarYUV(output, ptr);
                }
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
    fclose(output);
    return 0;
}
