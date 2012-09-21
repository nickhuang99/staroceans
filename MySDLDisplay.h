#ifndef MYSDLDISPLAY_H_INCLUDED
#define MYSDLDISPLAY_H_INCLUDED

#include <SDL/SDL.h>

class MySDLDisplay
{
protected:
    static SDL_Surface* screen;
    static SDL_Overlay* displayOverlay ;
    static SDL_Rect rect;
    static bool bReady;
    static unsigned int imgSize;
public:
    MySDLDisplay();

    unsigned int getImgSize() const ;

    bool init(unsigned int width=640, unsigned int height=480, unsigned int format=SDL_YUY2_OVERLAY);

    static bool displayImage(unsigned char* buffer);
    ~MySDLDisplay();

};

#endif // MYSDLDISPLAY_H_INCLUDED
