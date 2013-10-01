/*
 * MyX264.h
 *
 *  Created on: Sep 27, 2013
 *      Author: nick
 */

#ifndef MYX264_H_
#define MYX264_H_

/*
 * MyX264.cpp
 *
 *  Created on: Sep 26, 2013
 *      Author: nick
 */
#include <stdint.h>

extern "C"
{
	//#include <x264.h>
#include "x264.h"
}

using namespace std;


class MyX264
{
private:
	x264_param_t x264_param;
	x264_t * ptr_x264;
	x264_picture_t x264_pic;
	x264_nal_t * ptr_header;
	unsigned int m_width, m_height;
	FILE* output;

	bool init_param(x264_param_t*param);
	bool write_file(unsigned char* pData, unsigned int size);
	bool write_nal(x264_nal_t* p_nal, int i_nal);
	bool param_apply_profile( x264_param_t *param, const char *profile);
	bool param_apply_tune( x264_param_t *param, const char *s);
	bool param_apply_preset( x264_param_t *param, const char *preset);
	void copyPlanarYUV420(const unsigned char* srcPtr, unsigned char*yBuf, unsigned char* uBuf, unsigned char* vBuf);
	void copyPlanarYUV422(const unsigned char* srcPtr, unsigned char*yBuf, unsigned char* uBuf, unsigned char* vBuf);
public:
	MyX264()
	{
		ptr_x264 = NULL;
		output = NULL;
	}
	~MyX264();

	bool init(unsigned int width, unsigned int height, const char* fileName);

	bool encode_header();
	// assume it is yuv422 as input,
	bool encode_frame(unsigned char* ptr, unsigned int size);

};



#endif /* MYX264_H_ */
