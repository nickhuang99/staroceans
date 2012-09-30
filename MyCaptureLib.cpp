#include "MyCaptureLib.h"
#include "MyCapture.h"

static MyVideoCapture* pInstance = NULL;

extern "C" int myCaptureInit()
{
    if (pInstance == NULL)
    {
        pInstance = new MyVideoCapture();
    }
    if (pInstance == NULL)
    {
        return -1;
    }
    if (pInstance->startCapture())
    {
        return 0;
    }
    else
    {
        delete pInstance;
        pInstance = NULL;
        return -1;
    }
}

extern "C" int myCaptureSkip()
{
    if (pInstance)
    {
        unsigned char* unused[4];
        if (!pInstance->captureFrame(unused, true))
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

extern "C" int myCaptureRead(unsigned char* ptr[4])
{
    if (pInstance)
    {
        if (!pInstance->captureFrame(ptr))
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

extern "C" int myCaptureClose()
{
    if (pInstance)
    {
        delete pInstance;
        pInstance = NULL;
    }
    return 0;
}
