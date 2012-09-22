
#include "MySDLDisplay.h"

SDL_Surface* MySDLDisplay::screen;
SDL_Overlay* MySDLDisplay::displayOverlay ;
SDL_Rect MySDLDisplay::rect;
bool MySDLDisplay::bReady;
unsigned int MySDLDisplay::imgSize;

MySDLDisplay::MySDLDisplay()
{
    screen = NULL;
    displayOverlay = NULL;
    bReady = false;
    imgSize = 0;
}

unsigned int MySDLDisplay::getImgSize() const {return imgSize;}

bool MySDLDisplay::init(unsigned int width, unsigned int height, unsigned int format)
{
    if (bReady)
    {
        return true;
    }
    if ((screen = SDL_SetVideoMode(width, height, 24, SDL_SWSURFACE))!= NULL)
    {
        if ((displayOverlay = SDL_CreateYUVOverlay(width, height, format, screen)) != NULL)
        {
            if (displayOverlay->pitches[0] == width*2)
            {
                rect.x = rect.y = 0;
                rect.w = width;
                rect.h = height;
                imgSize = width*2*height;
                SDL_WM_SetCaption("My Simple SDL Display", NULL);
                bReady = true;

            }
            if (!bReady)
            {
                SDL_FreeYUVOverlay(displayOverlay);
            }
        }
        if (!bReady)
        {
            SDL_FreeSurface(screen);
        }
    }
    return bReady;
}


bool MySDLDisplay::displayImage(unsigned char* buffer)
{
    if (SDL_LockYUVOverlay(displayOverlay) == -1)
    {
        return false;
    }
    memcpy(displayOverlay->pixels[0], buffer, imgSize);

    SDL_UnlockYUVOverlay(displayOverlay);
    if (SDL_DisplayYUVOverlay(displayOverlay, &rect) == -1)
    {
        return false;
    }
    return true;
}
MySDLDisplay::~MySDLDisplay()
{
   if (bReady)
   {
       SDL_FreeYUVOverlay(displayOverlay);
       SDL_FreeSurface(screen);
       bReady = false;
   }
   SDL_Quit();
}
