/*****************************************************************************
 * Copyright (C) 2012 Javier Cabezas Gonzalez <jcabgz@gmail.com>
 *
 * This file is part of yuv2psnrssim.
 *
 * yuv2psnrssim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * yuv2psnrssim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with yuv2psnrssim. If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef _YUV2PSNRSSIM_H
#define _YUV2PSNRSSIM_H

#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>

#define MAX_NUM_PLANES 3
#define MAX_NUM_FRAMES 5000

// Colorspace defines
#define CSP_YUV420 1  /* YUV 4:2:0 planar */
#define CSP_YUV422 2  /* YUV 4:2:2 planar */
#define CSP_YUV444 3  /* YUV 4:4:4 planar */

// Y4M defines
#define Y4M_SIGNATURE "YUV4MPEG2"
#define MAX_Y4M_HEADER 80
#define Y4M_FRAME_SIGNATURE "FRAME"

// Decimal separator defines
#define DOT 1
#define COMMA 2

// File format defines
#define YUV 1
#define Y4M 2

// FIXME:     all singed type should be declared explicit,  for example,  int8_t should be declared as signed char.  
typedef signed char      int8_t  ;
typedef unsigned char    uint8_t ;
typedef short            int16_t ;
typedef unsigned short   uint16_t;
typedef int              int32_t ;
typedef unsigned int     uint32_t;
typedef __int64          int64_t ;
typedef unsigned __int64 uint64_t;

#ifdef _DEBUG
	#define EXIT_ON_ERROR(message)\
	{\
		printf_s(message);\
		system("PAUSE");\
		exit(0);\
	}
#else
	#define EXIT_ON_ERROR(message)\
	{\
		printf_s(message);\
		exit(0);\
	}
#endif

typedef struct yuv2psnrssimContext {

	/*****************************************************************************
	 * File related variables.
	 *****************************************************************************/

	/*******************************************************************
	 * Relative path to the original file as entered by the user.
	 *******************************************************************/
	char*     original_file_name;

	/*******************************************************************
	 * Relative path to the compressed file as entered by the user.
	 *******************************************************************/
	char*     compressed_file_name;

	/*******************************************************************
	 * Relative path to the .csv file as entered by the user.
	 *******************************************************************/
	char*     csv_file_name;

	/*******************************************************************
	 * Decimal separator for the .csv file. Excel expects different .csv
	 * formats depending on the decimal separator used by Windows,
	 * so we write the file differently based on that. Default is dot (.)
	 *******************************************************************/
	uint8_t   csv_decimal_separator;

	/*******************************************************************
	 * FILE pointer to the original file.
	 *******************************************************************/
	FILE*     original_file;

	/*******************************************************************
	 * FILE pointer to the compressed file.
	 *******************************************************************/
	FILE*     compressed_file;

	/*******************************************************************
	 * FILE pointer to the compressed file.
	 *******************************************************************/
	FILE*     csv_file;

	/*******************************************************************
	 * File format: YUV or Y4M. Both files must have the same format.
	 *******************************************************************/
	uint8_t   file_format;

	/*******************************************************************
	 * Size of the original file in bytes.
	 *******************************************************************/
	uint64_t  original_file_size_in_bytes;

	/*******************************************************************
	 * Size of the compressed file in bytes.
	 *******************************************************************/
	uint64_t  compressed_file_size_in_bytes;





	/*****************************************************************************
	 * Video related variables.
	 *****************************************************************************/

	/*******************************************************************
	 * Video width. Both files must have the same width.
	 *******************************************************************/
	uint32_t  width;

	/*******************************************************************
	 * Video height. Both files must have the same height.
	 *******************************************************************/
	uint32_t  height;

	/*******************************************************************
	 * Colorspace: YUV 4:2:0 planar, YUV 4:2:2 planar or YUV 4:4:4 planar.
	 * Both files must have the same colorspace. Default is YUV 4:2:0.
	 *******************************************************************/
	uint8_t   colorspace;

	/*******************************************************************
	 * Specifies the number of bits per pixel of the original file.
	 * If this value is > 8, yuv2psnrssim will read two bytes of
	 * input data for each pixel sample, and expect the upper
	 * (16 - original_bit_depth) bits to be zero. Default is 8.
	 *******************************************************************/
	uint8_t   original_bit_depth;

	/*******************************************************************
	 * Specifies the number of bits per pixel of the compressed file.
	 * If this value is > 8, yuv2psnrssim will read two bytes of
     * input data for each pixel sample, and expect the upper
	 * (16 - compressed_bit_depth) bits to be zero. Default is 8.
	 *******************************************************************/
	uint8_t   compressed_bit_depth;

	/*******************************************************************
	 * Specifies the maximum number of bits per pixel of the two files.
	 * For example, if original_bit_depth is 8 and compressed_bit_depth
	 * is 10, max_bit_depth will be 10. This variable tells us whether
	 * we have to upconvert a plane to a higher bit depth for processing
	 * or not.
	 * In the previous example, where original_bit_depth is 8,
	 * compressed_bit_depth is 10 and max_bit_depth is 10, the original
	 * pixels will be shifted to the left (max_bit_depth - original_bit_depth)
	 * or 10 - 8 = 2 positions, equivalent of multiplying by 4. Compressed
	 * pixels will pass untouched, since they already have max_bit_depth
	 * meaningful bits. This is needed when dealing with files with
	 * different bit depths.
	 *******************************************************************/
	uint8_t   max_bit_depth;

	/*******************************************************************
	 * Size of each plane in pixels. Chroma plane size depends on colorspace.
	 *******************************************************************/
	uint32_t  plane_size_in_pixels[MAX_NUM_PLANES];

	/*******************************************************************
	 * Size of the frames in the original file.
	 *******************************************************************/
	uint64_t  original_frame_size_in_bytes;

	/*******************************************************************
	 * Size of the frames in the compressed file.
	 *******************************************************************/
	uint64_t  compressed_frame_size_in_bytes;

	/*******************************************************************
	 * Number of frames in the files. Each file can have a different size,
	 * bit depth, etc. but the number of frames must be the same in both.
	 *******************************************************************/
	uint64_t  num_frames;

	/*******************************************************************
	 * Pointers to the planes of each frame of the original file. By design,
	 * one frame is read and processed. Then the following frame is read,
	 * overwriting the previous one, and processed. And so on until the
	 * end of the file. Pixels are stored internally as uint16_t (16 bit
	 * unsigned integers), performing upconversion if necessary. See
	 * max_bit_depth description for more details.
	 *******************************************************************/
	uint16_t* original_planes[MAX_NUM_PLANES];

	/*******************************************************************
	 * Pointers to the planes of each frame of the compressed file. By design,
	 * one frame is read and processed. Then the following frame is read,
	 * overwriting the previous one, and processed. And so on until the
	 * end of the file. Pixels are stored internally as uint16_t (16 bit
	 * unsigned integers), performing upconversion if necessary. See
	 * max_bit_depth description for more details.
	 *******************************************************************/
	uint16_t* compressed_planes[MAX_NUM_PLANES];

	/*******************************************************************
	 * Pointers to temporal 8 bit per pixel planes. When the bit depth is
	 * 8 or less frame data is temporarily stored in these buffers before
	 * being upconverted if necessary. Since we will probably have to
	 * upconvert the input to a higher bit depth, there are two options:
	 * - Read pixel by pixel, and for each one upconvert and store in the
	 *   definitive buffers.
	 * - Read whole planes in a single fread call to these buffers,
	 *   upconvert each pixel with a loop and store in the definitive buffers.
	 * The second option is much better for speed reasons, because reading
	 * files byte by byte has an enormous overhead.
	 *******************************************************************/
	uint8_t*  temp_planes[MAX_NUM_PLANES];





	/*****************************************************************************
	 * Y4M related variables.
	 *****************************************************************************/

	/*******************************************************************
	 * Y4M original header size in bytes.
	 *******************************************************************/
	uint8_t   Y4M_original_header_size_in_bytes;

	/*******************************************************************
	 * Y4M compressed header size in bytes.
	 *******************************************************************/
	uint8_t   Y4M_compressed_header_size_in_bytes;

	/*******************************************************************
	 * Y4M frame header size in bytes. This header appears before each
	 * frame in the file.
	 *******************************************************************/
	uint8_t   Y4M_frame_header_size_in_bytes;





	/*****************************************************************************
	 * Quality metrics related variables.
	 *****************************************************************************/

	/*******************************************************************
	 * Array used to store MSE frame and plane data. An aditional value
	 * (combined MSE) is stored for each frame.
	 *******************************************************************/
	double    MSE_array[MAX_NUM_FRAMES][MAX_NUM_PLANES + 1];

	/*******************************************************************
	 * Array used to store PSNR frame and plane data. An aditional value
	 * (combined PSNR) is stored for each frame.
	 *******************************************************************/
	double    PSNR_array[MAX_NUM_FRAMES][MAX_NUM_PLANES + 1];

	/*******************************************************************
	 * Value of PSNR mean Y.
	 *******************************************************************/
	double    PSNR_meanY;

	/*******************************************************************
	 * Value of PSNR mean Cb.
	 *******************************************************************/
	double    PSNR_meanCb;

	/*******************************************************************
	 * Value of PSNR mean Cr.
	 *******************************************************************/
	double    PSNR_meanCr;

	/*******************************************************************
	 * Value of combined PSNR.
	 *******************************************************************/
	double    PSNR_combined;

	/*******************************************************************
	 * Array used to store SSIM Y frame data.
	 *******************************************************************/
	double    SSIM_array[MAX_NUM_FRAMES][MAX_NUM_PLANES ];

	/*******************************************************************
	 * Value of SSIM mean Y.
	 *******************************************************************/
	double    SSIM_meanY;
	double    SSIM_meanCb;
	double    SSIM_meanCr;

	double    SSIM_combined;

	/*****************************************************************************
	 * Misc.
	 *****************************************************************************/

	/*******************************************************************
	 * Time taken by the program from start to finish. In seconds.
	 *******************************************************************/
	double    time;

	/*******************************************************************
	 * This variable controls whether results are printed to the console
	 * or not. Default is yes (1).
	 *******************************************************************/
	uint8_t   print_results;

} yuv2psnrssimContext;

#endif