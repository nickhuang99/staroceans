#ifndef MYCAPTURELIB_H_INCLUDED
#define MYCAPTURELIB_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int myCaptureInit();
int myCaptureSkip();
int myCaptureRead(unsigned char*ptr[4]);
int myCaptureClose();

#ifdef __cplusplus
}
#endif


#endif // MYCAPTURELIB_H_INCLUDED
