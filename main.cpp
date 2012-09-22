#include <iostream>
#include "MyCapture.h"
#include "MySDLDisplay.h"


using namespace std;


int main()
{

    MyVideoCapture capture;
    MySDLDisplay display;

    //capture.startCaptureAndDisplay();

    size_t size = capture.startCapture();
    if (size > 0 && display.init() && display.getImgSize() == size)
    {
        unsigned char* ptr = new unsigned char[size];
        if (ptr)
        {
            int counter = 0;
            struct timeval last, now;
            while (counter <300)
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

    return 0;
}
