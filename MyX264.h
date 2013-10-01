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
#include <x264.h>
}

using namespace std;

/* Analyse flags
 */
#define X264_ANALYSE_I4x4       0x0001  /* Analyse i4x4 */
#define X264_ANALYSE_I8x8       0x0002  /* Analyse i8x8 (requires 8x8 transform) */
#define X264_ANALYSE_PSUB16x16  0x0010  /* Analyse p16x8, p8x16 and p8x8 */
#define X264_ANALYSE_PSUB8x8    0x0020  /* Analyse p8x4, p4x8, p4x4 */
#define X264_ANALYSE_BSUB16x16  0x0100  /* Analyse b16x8, b8x16 and b8x8 */
#define X264_DIRECT_PRED_NONE        0
#define X264_DIRECT_PRED_SPATIAL     1
#define X264_DIRECT_PRED_TEMPORAL    2
#define X264_DIRECT_PRED_AUTO        3
#define X264_ME_DIA                  0
#define X264_ME_HEX                  1
#define X264_ME_UMH                  2
#define X264_ME_ESA                  3
#define X264_ME_TESA                 4
#define X264_CQM_FLAT                0
#define X264_CQM_JVT                 1
#define X264_CQM_CUSTOM              2
#define X264_RC_CQP                  0
#define X264_RC_CRF                  1
#define X264_RC_ABR                  2
#define X264_QP_AUTO                 0
#define X264_AQ_NONE                 0
#define X264_AQ_VARIANCE             1
#define X264_AQ_AUTOVARIANCE         2
#define X264_B_ADAPT_NONE            0
#define X264_B_ADAPT_FAST            1
#define X264_B_ADAPT_TRELLIS         2
#define X264_WEIGHTP_NONE            0
#define X264_WEIGHTP_SIMPLE          1
#define X264_WEIGHTP_SMART           2
#define X264_B_PYRAMID_NONE          0
#define X264_B_PYRAMID_STRICT        1
#define X264_B_PYRAMID_NORMAL        2
#define X264_KEYINT_MIN_AUTO         0
#define X264_KEYINT_MAX_INFINITE     (1<<30)

class MyX264
{
private:
	x264_param_t x264_param;
	x264_t * ptr_x264;
	x264_picture_t x264_pic;
	x264_nal_t * ptr_header;
	unsigned int m_width, m_height;
	FILE* output;

	bool init_param(x264_param_t*param, unsigned long int chromeFormat);
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

	bool init(unsigned int width, unsigned int height, const char* fileName, unsigned long int chromeFormat = X264_CSP_I420);

	bool encode_header();
	// assume it is yuv422 as input,
	bool encode_frame(unsigned char* ptr, unsigned int size);

};



#endif /* MYX264_H_ */
