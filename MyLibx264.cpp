/*
 * MyLibx264.cpp
 *
 *  Created on: Sep 26, 2013
 *      Author: nick
 */

#include <cstdio>
#include <cstring>
#include "MyLibx264.h"

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


MyLibx264::~MyLibx264()
{

}

void MyLibx264::uninit()
{
	x264_picture_t pic_out;
	int i_frame_size = 0;
	x264_nal_t* p_nal = NULL;
	int i_nal = 0;
	while (x264_encoder_delayed_frames(ptr_x264))
	{
		i_frame_size = x264_encoder_encode(ptr_x264, &p_nal, &i_nal, NULL, &pic_out);
		if (i_frame_size > 0)
		{
			printf("********delayed*************frame %ld*******delayed*********\n", pic_out.i_pts);
			write_nal(p_nal, i_nal);
		}
	}
	x264_picture_clean(&x264_pic);
	x264_encoder_close(ptr_x264);
	if (output)
	{
		fclose(output);
	}
}

bool MyLibx264::init_param(x264_param_t*param, unsigned long int chromeFormat)
{
	x264_param_default(param);
	// preset of "ultrafast"
	if (!param_apply_preset(param, "faster"))
	{
		return false;
	}
	// tune of "film"
	if (!param_apply_tune(param, "film"))
	{
		return false;
	}

	// profile "baseline"
	if (!param_apply_profile(param, "main"))
	{
		return false;
	}
	// setup csp
	param->i_csp = chromeFormat;
	param->i_width = m_width;
	param->i_height = m_height;
	return true;
}

// FIX-ME: all error handling...
bool MyLibx264::init(const char* fileName, unsigned int width, unsigned int height,  unsigned long int chromeFormat)
{
	m_width = width;
	m_height = height;
	if (!init_param(&x264_param, chromeFormat))
	{
		printf("init param failed!\n");
		return false;
	}
	ptr_x264 = x264_encoder_open( &x264_param);

	if (ptr_x264 == NULL)
	{
		printf("cannot open encoder!\n");
		return false;
	}

	if (x264_picture_alloc( &x264_pic, x264_param.i_csp, width, height)!= 0)
	{
		return false;
	}
	x264_pic.param = &x264_param;
	output = fopen(fileName, "w+b");
	if (output == NULL)
	{
		printf("cannot open file %s\n", fileName);
		return false;
	}
	return true;
}
bool MyLibx264::write_file(unsigned char* pData, unsigned int size)
{
	unsigned int total_written_size = 0, current_size_to_write = size;
	while (total_written_size < size)
	{
		current_size_to_write = size - total_written_size;
		int write_size = fwrite(pData, 1, current_size_to_write, output);
		if (write_size <= -1)
		{
			return false;
		}
		total_written_size += write_size;
		// debugging only
		if (total_written_size != size)
		{
			printf("****************file writing delayed!***************\n");
		}
	}
	return true;
}

bool MyLibx264::write_nal(x264_nal_t* p_nal, int i_nal)
{
	for (int i = 0; i < i_nal; i ++)
	{
		 if (!write_file(p_nal[i].p_payload, p_nal[i].i_payload))
		 {
			 return false;
		 }
	}
	return true;
}

bool MyLibx264::encode_header()
{
	x264_nal_t* p_nal;
	int i_nal = 0;
	if (x264_encoder_headers(ptr_x264, &p_nal, &i_nal) < 0)
	{
		return false;
	}
	return write_nal(p_nal, i_nal);
}

void MyLibx264::copyPlanarYUV420(const unsigned char* srcPtr, unsigned char*yBuf, unsigned char* uBuf, unsigned char* vBuf)
{
	const unsigned char* ptr = srcPtr;
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < m_height; r ++)
	{
		if (r % 2 == 0)
		{
			for (unsigned int c = 0; c < m_width/2; c ++)
			{
				*yPtr ++ = *ptr ++;
				*uPtr ++ = *ptr ++;
				*yPtr ++ = *ptr ++;
				*vPtr ++ = *ptr ++;
			}
		}
		else
		{
			for (unsigned int c = 0; c < m_width/2; c ++)
			{
				*yPtr ++ = *ptr ++;
				ptr ++;
				*yPtr ++ = *ptr ++;
				ptr ++;
			}
		}

	}
}

void MyLibx264::copyPlanarYUV422(const unsigned char* srcPtr, unsigned char*yBuf, unsigned char* uBuf, unsigned char* vBuf)
{
	const unsigned char* ptr = srcPtr;
	unsigned char* yPtr = yBuf, * uPtr = uBuf, * vPtr= vBuf;
	for (unsigned int r = 0; r < m_height; r ++)
	{
		for (unsigned int c = 0; c < m_width/2; c ++)
		{
			*yPtr ++ = *ptr ++;
			*uPtr ++ = *ptr ++;
			*yPtr ++ = *ptr ++;
			*vPtr ++ = *ptr ++;
		}
	}
}
// assume it is yuv422 as input,
bool MyLibx264::encode_frame(unsigned char* ptr)
{
	static bool bFirst = true;
	static int64_t i_pts = 1;
	x264_image_t& img = x264_pic.img;

	//img.i_csp = X264_CSP_I422;
	img.i_plane = 3;

	img.i_stride[0] = m_width;
	img.i_stride[1] = m_width/2;
	img.i_stride[2] = m_width/2;
	if (bFirst)
	{
		bFirst = false;
		encode_header();
	}
	unsigned char* yPtr = img.plane[0];
	unsigned char* uPtr = img.plane[1];
	unsigned char* vPtr = img.plane[2];
	if (x264_param.i_csp == X264_CSP_I420)
	{
		//copyPlanarYUV420(ptr, yPtr, uPtr, vPtr);
		unsigned int size = m_width*m_height;
		memcpy(yPtr, ptr, size);
		memcpy(uPtr, ptr + size, size/4);
		memcpy(vPtr, ptr + size*5/4, size/4);
	}
	else
	{
		if (x264_param.i_csp == X264_CSP_I422)
		{
			//copyPlanarYUV422(ptr, yPtr, uPtr, vPtr);
			//later
		}
	}

	x264_picture_t pic_out;
	int i_frame_size = 0;
	x264_nal_t* p_nal = NULL;
	int i_nal = 0;
	x264_pic.i_pts = i_pts ++;
	i_frame_size = x264_encoder_encode(ptr_x264, &p_nal, &i_nal, &x264_pic, &pic_out);
	if (i_frame_size > 0)
	{
		//i_pts = pic_out.i_pts;
		printf("***************************frame %ld*********************\n", pic_out.i_pts);
		return write_nal(p_nal, i_nal);
	}
	//x264_pic.i_pts ++;
	return true;
}

bool MyLibx264::param_apply_preset( x264_param_t *param, const char *preset)
{
    if( !strcasecmp( preset, "ultrafast" ) )
    {
        param->i_frame_reference = 1;
        param->i_scenecut_threshold = 0;
        param->b_deblocking_filter = 0;
        param->b_cabac = 0;
        param->i_bframe = 0;
        param->analyse.intra = 0;
        param->analyse.inter = 0;
        param->analyse.b_transform_8x8 = 0;
        param->analyse.i_me_method = X264_ME_DIA;
        param->analyse.i_subpel_refine = 0;
        param->rc.i_aq_mode = 0;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->i_bframe_adaptive = X264_B_ADAPT_NONE;
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
        param->analyse.b_weighted_bipred = 0;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "superfast" ) )
    {
        param->analyse.inter = X264_ANALYSE_I8x8|X264_ANALYSE_I4x4;
        param->analyse.i_me_method = X264_ME_DIA;
        param->analyse.i_subpel_refine = 1;
        param->i_frame_reference = 1;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->rc.b_mb_tree = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 0;
    }
    else if( !strcasecmp( preset, "veryfast" ) )
    {
        param->analyse.i_me_method = X264_ME_HEX;
        param->analyse.i_subpel_refine = 2;
        param->i_frame_reference = 1;
        param->analyse.b_mixed_references = 0;
        param->analyse.i_trellis = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 10;
    }
    else if( !strcasecmp( preset, "faster" ) )
    {
        param->analyse.b_mixed_references = 0;
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 4;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 20;
    }
    else if( !strcasecmp( preset, "fast" ) )
    {
        param->i_frame_reference = 2;
        param->analyse.i_subpel_refine = 6;
        param->analyse.i_weighted_pred = X264_WEIGHTP_SIMPLE;
        param->rc.i_lookahead = 30;
    }
    else if( !strcasecmp( preset, "medium" ) )
    {
        /* Default is medium */
    }
    else if( !strcasecmp( preset, "slow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 8;
        param->i_frame_reference = 5;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->rc.i_lookahead = 50;
    }
    else if( !strcasecmp( preset, "slower" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 9;
        param->i_frame_reference = 8;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->rc.i_lookahead = 60;
    }
    else if( !strcasecmp( preset, "veryslow" ) )
    {
        param->analyse.i_me_method = X264_ME_UMH;
        param->analyse.i_subpel_refine = 10;
        param->analyse.i_me_range = 24;
        param->i_frame_reference = 16;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.i_trellis = 2;
        param->i_bframe = 8;
        param->rc.i_lookahead = 60;
    }
    else if( !strcasecmp( preset, "placebo" ) )
    {
        param->analyse.i_me_method = X264_ME_TESA;
        param->analyse.i_subpel_refine = 11;
        param->analyse.i_me_range = 24;
        param->i_frame_reference = 16;
        param->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
        param->analyse.i_direct_mv_pred = X264_DIRECT_PRED_AUTO;
        param->analyse.inter |= X264_ANALYSE_PSUB8x8;
        param->analyse.b_fast_pskip = 0;
        param->analyse.i_trellis = 2;
        param->i_bframe = 16;
        param->rc.i_lookahead = 60;
    }
    else
    {
        printf("invalid preset '%s'\n", preset );
        return false;
    }
    return true;
}

bool MyLibx264::param_apply_tune( x264_param_t *param, const char *s )
{
	if( !strncasecmp( s, "film", 4 ) )
	{
		param->i_deblocking_filter_alphac0 = -1;
		param->i_deblocking_filter_beta = -1;
		param->analyse.f_psy_trellis = 0.15;
	}
	else if( !strncasecmp( s, "animation", 9 ) )
	{
		param->i_frame_reference = param->i_frame_reference > 1 ? param->i_frame_reference*2 : 1;
		param->i_deblocking_filter_alphac0 = 1;
		param->i_deblocking_filter_beta = 1;
		param->analyse.f_psy_rd = 0.4;
		param->rc.f_aq_strength = 0.6;
		param->i_bframe += 2;
	}
	else if( !strncasecmp( s, "grain", 5 ) )
	{
		param->i_deblocking_filter_alphac0 = -2;
		param->i_deblocking_filter_beta = -2;
		param->analyse.f_psy_trellis = 0.25;
		param->analyse.b_dct_decimate = 0;
		param->rc.f_pb_factor = 1.1;
		param->rc.f_ip_factor = 1.1;
		param->rc.f_aq_strength = 0.5;
		param->analyse.i_luma_deadzone[0] = 6;
		param->analyse.i_luma_deadzone[1] = 6;
		param->rc.f_qcompress = 0.8;
	}
	else if( !strncasecmp( s, "stillimage", 5 ) )
	{
		param->i_deblocking_filter_alphac0 = -3;
		param->i_deblocking_filter_beta = -3;
		param->analyse.f_psy_rd = 2.0;
		param->analyse.f_psy_trellis = 0.7;
		param->rc.f_aq_strength = 1.2;
	}
	else if( !strncasecmp( s, "psnr", 4 ) )
	{
		param->rc.i_aq_mode = X264_AQ_NONE;
		param->analyse.b_psy = 0;
	}
	else if( !strncasecmp( s, "ssim", 4 ) )
	{
		param->rc.i_aq_mode = X264_AQ_AUTOVARIANCE;
		param->analyse.b_psy = 0;
	}
	else if( !strncasecmp( s, "fastdecode", 10 ) )
	{
		param->b_deblocking_filter = 0;
		param->b_cabac = 0;
		param->analyse.b_weighted_bipred = 0;
		param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
	}
	else if( !strncasecmp( s, "zerolatency", 11 ) )
	{
		param->rc.i_lookahead = 0;
		param->i_sync_lookahead = 0;
		param->i_bframe = 0;
		param->b_sliced_threads = 1;
		param->b_vfr_input = 0;
		param->rc.b_mb_tree = 0;
	}
	else if( !strncasecmp( s, "touhou", 6 ) )
	{
		param->i_frame_reference = param->i_frame_reference > 1 ? param->i_frame_reference*2 : 1;
		param->i_deblocking_filter_alphac0 = -1;
		param->i_deblocking_filter_beta = -1;
		param->analyse.f_psy_trellis = 0.2;
		param->rc.f_aq_strength = 1.3;
		if( param->analyse.inter & X264_ANALYSE_PSUB16x16 )
		{
			param->analyse.inter |= X264_ANALYSE_PSUB8x8;
		}
	}
	else
	{
		printf("invalid tune '%s'\n", s );
		return false;
	}
    return true;
}

bool MyLibx264::param_apply_profile( x264_param_t *param, const char *profile )
{

    if( ((param->rc.i_rc_method == X264_RC_CQP && param->rc.i_qp_constant <= 0) ||
        (param->rc.i_rc_method == X264_RC_CRF && (int)(param->rc.f_rf_constant) <= 0)) )
    {
        printf("%s profile doesn't support lossless\n", profile );
        return false;
    }

    if( strncasecmp("base", profile, 4) == 0)
    {
        param->analyse.b_transform_8x8 = 0;
        param->b_cabac = 0;
        param->i_cqm_preset = X264_CQM_FLAT;
        param->psz_cqm_file = NULL;
        param->i_bframe = 0;
        param->analyse.i_weighted_pred = X264_WEIGHTP_NONE;
        if( param->b_interlaced )
        {
            printf("baseline profile doesn't support interlacing\n" );
            return false;
        }

        return true;
    }

	if( strncasecmp("main", profile, 4) == 0 )
	{
		param->analyse.b_transform_8x8 = 0;
		param->i_cqm_preset = X264_CQM_FLAT;
		param->psz_cqm_file = NULL;
		return true;
	}
    return false;
}
