/*
 * mylibx264.h
 *
 *  Created on: Oct 6, 2013
 *      Author: nick
 */

#ifndef MYLIBX264_H_
#define MYLIBX264_H_

#include <stdint.h>
#include <cstdio>

using namespace std;
extern "C"
{
#include <x264.h>
}

using namespace std;

class MyLibx264
{
private:
	x264_param_t x264_param;
	x264_t * ptr_x264;
	x264_picture_t x264_pic;
	x264_nal_t * ptr_header;
	unsigned int m_width, m_height;
	FILE* output;

	bool init_param(x264_param_t*param, unsigned long int chromeFormat);
	bool write_file(uint8_t* pData, unsigned int size);
	bool write_nal(x264_nal_t* p_nal, int i_nal);
	bool param_apply_profile( x264_param_t *param, const char *profile);
	bool param_apply_tune( x264_param_t *param, const char *s);
	bool param_apply_preset( x264_param_t *param, const char *preset);
	void copyPlanarYUV420(const uint8_t* srcPtr, uint8_t*yBuf, uint8_t* uBuf, uint8_t* vBuf);
	void copyPlanarYUV422(const uint8_t* srcPtr, uint8_t*yBuf, uint8_t* uBuf, uint8_t* vBuf);
public:
	MyLibx264()
	{
		ptr_x264 = NULL;
		output = NULL;
	}
	~MyLibx264();
	void uninit();

	bool init(const char* fileName, unsigned int width, unsigned int height, unsigned long int chromeFormat = X264_CSP_I420);

	bool encode_header();
	// assume it is yuv422 as input,
	bool encode_frame(uint8_t* ptr);

};

#endif /* MYLIBX264_H_ */
