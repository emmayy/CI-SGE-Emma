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

#include <string.h>
#include <time.h>
#include "yuv2psnrssim.h"
#include "ssim.h"
#include "psnr.h"

/***********************************************************************************
 * This function initializes the program context variables to their default value.
 ***********************************************************************************/
static void initialize_context(yuv2psnrssimContext* context)
{
	int i, j;

	context->original_file_name = NULL;
	context->compressed_file_name = NULL;
	context->csv_file_name = NULL;
	context->csv_decimal_separator = DOT;
	context->original_file = NULL;
	context->compressed_file = NULL;
	context->csv_file = NULL;
	context->file_format = YUV;
	context->original_file_size_in_bytes = 0;
	context->compressed_file_size_in_bytes = 0;

	context->width = 0;
	context->height = 0;
	context->colorspace = CSP_YUV420;
	context->original_bit_depth = 8;
	context->compressed_bit_depth = 8;
	context->max_bit_depth = 0;
	for (i = 0; i < MAX_NUM_PLANES; i++)
		context->plane_size_in_pixels[i] = 0;
	context->original_frame_size_in_bytes = 0;
	context->compressed_frame_size_in_bytes = 0;
	context->num_frames = 0;
	for (i = 0; i < MAX_NUM_PLANES; i++)
		context->original_planes[i] = NULL;
	for (i = 0; i < MAX_NUM_PLANES; i++)
		context->compressed_planes[i] = NULL;
	for (i = 0; i < MAX_NUM_PLANES; i++)
		context->temp_planes[i] = NULL;

	context->Y4M_original_header_size_in_bytes = 0;
	context->Y4M_compressed_header_size_in_bytes = 0;
	context->Y4M_frame_header_size_in_bytes = 0;

	for (i = 0; i < MAX_NUM_FRAMES; i++)
		for (j = 0; j < MAX_NUM_PLANES + 1; j++)
			context->MSE_array[i][j] = 0;
	for (i = 0; i < MAX_NUM_FRAMES; i++)
		for (j = 0; j < MAX_NUM_PLANES + 1; j++)
			context->PSNR_array[i][j] = 0;
	context->PSNR_meanY = 0;
	context->PSNR_meanCb = 0;
	context->PSNR_meanCr = 0;
	context->PSNR_combined = 0;
	for (i = 0; i < MAX_NUM_FRAMES; i++)
		for (j = 0; j < MAX_NUM_PLANES ; j++)
		   context->SSIM_array[i][j] = 0;
	context->SSIM_meanY = 0;
	context->SSIM_meanCb = 0;
	context->SSIM_meanCr = 0;
	context->SSIM_combined = 0;
 

	context->time = 0;
	context->print_results = 1;
}

/***********************************************************************************
 * This function parses the command line passed to the program. It also prints
 * help and error messages when necessary.
 ***********************************************************************************/
static void parse(int argc, char* argv[], yuv2psnrssimContext* context)
{
	char* lastdot;

	if (argc <= 2) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
		switch(argc)
		{
			case 1: // Inform the user of how to use the program
				printf_s("\n\nBasic usage is -orig <original file> -comp <compressed file> -res <resolution>\n\n");
				printf_s("For more detailed help, use the -help switch\n\n");
				break;
			case 2: // Inform the user of how to use the program in detail
				if (!strcmp(argv[1], "-help"))
					printf_s("\n\nyuv2psnrssim calculates PSNR and SSIM objetive quality metrics of two given\n"
					"uncompressed video sequences. These sequences can be either in uncompressed\n"
					"YUV format or in uncompressed Y4M format. Both formats hold the same\n"
					"video information, Y4M having an internal header used to store useful\n"
					"metadata like resolution, colorspace, frame rate, etc.\n\n"
					"yuv2psnrssim supports up to 16 bits per sample, 4:2:0, 4:2:2 and 4:4:4\n"
					"chroma subsampling schemes and can output results in the Excel friendly\n"
					"CSV format.\n\n\n"
					"Example usage:\n\n"
					"    -orig <original file> -comp <compressed file> [other options]\n\n\n"
					"Options:\n\n"
					"    -o, -orig, -original <string>      Relative path to the original file.\n"
					"                                       Required.\n\n"
					"    -c, -comp, -compressed <string>    Relative path to the compressed file.\n"
					"                                       Required.\n\n"
					"    -r, -res, -resolution <intxint>    Resolution of the two files. They must\n"
					"                                       be the same. Required for YUV input.\n"
					"                                       Format: widthxheight\n\n"
					"    -csp, -colorspace <string>         Colorspace of the two files. They must\n"
					"                                       be the same. It can be yuv420, yuv422 or\n"
					"                                       yuv444. Default is yuv420.\n\n"
					"    -ob, -orig_bit_depth <integer>     Sample bit depth of the original file.\n"
					"                                       Default is 8 bits.\n\n"
					"    -cb, -comp_bit_depth <integer>     Sample bit depth of the compressed file.\n"
					"                                       Default is 8 bits.\n\n"
					"    -csv_output_file <string>          Relative path to the .csv results file.\n\n"
					"    -csv_decimal_separator <string>    Decimal separator used in the .csv file.\n"
					"                                       Use \"comma\" or \",\" in European regions,\n"
					"                                       \"dot\" or \".\" in American regions.\n"
					"                                       Default is \".\" dot.\n\n"
					"    -no_results                        Don't print results to the console.\n\n");
				else
					printf_s("Invalid arguments, please try again.\n\n");
		}
		#ifdef _DEBUG
			system("PAUSE");
		#endif
		exit(0);
	} else {
		int original_file_format;
		int compressed_file_format;
		int i;

		for (i = 1; i < argc; i++) {
			if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "-orig") || !strcmp(argv[i], "-original")) {
				context->original_file_name = argv[i + 1];
				lastdot = strrchr(context->original_file_name, '.');
				if (lastdot != NULL)
				{
					if (!strcmp(lastdot, ".yuv"))
						original_file_format = YUV;
					else if (!strcmp(lastdot, ".y4m"))
						original_file_format = Y4M;
					else
						EXIT_ON_ERROR("\n\nERROR -- Unable to retrieve original file format.\n\n");
				}
				else
					EXIT_ON_ERROR("\n\nERROR -- Unable to retrieve original file format.\n\n");
				i++;
			} else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "-comp") || !strcmp(argv[i], "-compressed")) {
				context->compressed_file_name = argv[i + 1];
				lastdot = strrchr(context->compressed_file_name, '.');
				if (lastdot != NULL)
				{
					if (!strcmp(lastdot, ".yuv"))
						compressed_file_format = YUV;
					else if (!strcmp(lastdot, ".y4m"))
						compressed_file_format = Y4M;
					else
						EXIT_ON_ERROR("\n\nERROR -- Unable to retrieve compressed file format.\n\n");
				}
				else
					EXIT_ON_ERROR("\n\nERROR -- Unable to retrieve compressed file format.\n\n");
				i++;
			} else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "-res") || !strcmp(argv[i], "-resolution")) {
				sscanf_s(argv[i + 1], "%ux%u", &context->width, &context->height);
				i++;
			} else if (!strcmp(argv[i], "-csp") || !strcmp(argv[i], "-colorspace")) {
				if (!strcmp(argv[i + 1], "yuv420"))
					context->colorspace = CSP_YUV420;
				else if (!strcmp(argv[i + 1], "yuv422"))
					context->colorspace = CSP_YUV422;
				else if (!strcmp(argv[i + 1], "yuv444"))
					context->colorspace = CSP_YUV444;
				else
					EXIT_ON_ERROR("\n\nERROR -- Unsupported colorspace. Valid options are yuv420, yuv422 and yuv444.\n\n");
				i++;
			} else if (!strcmp(argv[i], "-ob") || !strcmp(argv[i], "orig_bit_depth")) {
				context->original_bit_depth = atoi(argv[i + 1]);
				i++;
			} else if (!strcmp(argv[i], "-cb") || !strcmp(argv[i], "comp_bit_depth")) {
				context->compressed_bit_depth = atoi(argv[i + 1]);
				i++;
			} else if (!strcmp(argv[i], "-csv_output_file")) {
				context->csv_file_name = argv[i + 1];
				i++;
			} else if (!strcmp(argv[i], "-csv_decimal_separator")) {
				if (!strcmp(argv[i + 1], "comma") || !strcmp(argv[i + 1], ","))
					context->csv_decimal_separator = COMMA;
				else if (!strcmp(argv[i + 1], "dot") || !strcmp(argv[i + 1], "."))
					context->csv_decimal_separator = DOT;
				else
					EXIT_ON_ERROR("\n\nERROR -- Unsupported decimal separator.\n\n");
				i++;
			} else if (!strcmp(argv[i], "-no_results")) {
				context->print_results = 0;
			} else
				EXIT_ON_ERROR("\n\nERROR -- Invalid arguments, please try again.\n\n");
		}

		if (context->original_file_name == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Original file is missing. You have to specify both original\nand compressed files.\n\n");
		if (context->compressed_file_name == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Compressed file is missing. You have to specify both original\nand compressed files\n\n");

		if (original_file_format != compressed_file_format)
		{
			EXIT_ON_ERROR("\n\nERROR -- File formats of original and compressed files do not match.\n\n");
		}
		else
			context->file_format = original_file_format;

		if (context->file_format == YUV && (context->width == 0 || context->height == 0))
			EXIT_ON_ERROR("\n\nERROR -- When using .yuv files as input, you have to specify a resolution.\n\n");

		// Limit resolution to 8K for the time being
		if (context->width > 7680 || context->height > 4320)
			EXIT_ON_ERROR("\n\nERROR -- Unsupported resolution. Maximum allowed resolution is 7680x4320 (8K).\n\n");

		if (context->original_bit_depth < 8 || context->original_bit_depth > 16)
			EXIT_ON_ERROR("\n\nERROR -- The original file bit depth is unsupported.\nBit depth should be in the 8-16 range.\n\n");
		if (context->compressed_bit_depth < 8 || context->compressed_bit_depth > 16)
			EXIT_ON_ERROR("\n\nERROR -- The compressed file bit depth is unsupported.\nBit depth should be in the 8-16 range.\n\n");

		if (context->print_results == 0 && context->csv_file_name == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Result printing and .csv output are disabled.\nThe program won't do anything.\n\n");
	}
}

/***********************************************************************************
 * This function opens the input files in binary read/binary write mode.
 ***********************************************************************************/
static void open_files(yuv2psnrssimContext* context)
{
	fopen_s(&context->original_file, context->original_file_name, "rb");
	if (context->original_file == NULL)
		EXIT_ON_ERROR("\n\nERROR -- Failed to open original file.\n\n");

	fopen_s(&context->compressed_file, context->compressed_file_name, "rb");
	if (context->compressed_file == NULL)
		EXIT_ON_ERROR("\n\nERROR -- Failed to open compressed file.\n\n");

	if (context->csv_file_name != NULL)
	{
		fopen_s(&context->csv_file, context->csv_file_name, "wb");
		if (context->csv_file == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Failed to create csv file.\n\n");
	}
}

/***********************************************************************************
 * This function calculates the size of the input files.
 ***********************************************************************************/
static void get_file_sizes(yuv2psnrssimContext* context)
{
	_fseeki64(context->original_file, (int64_t)0, SEEK_END);
	context->original_file_size_in_bytes = _ftelli64(context->original_file);
	_fseeki64(context->original_file, (int64_t)0, SEEK_SET);

	_fseeki64(context->compressed_file, (int64_t)0, SEEK_END);
	context->compressed_file_size_in_bytes = _ftelli64(context->compressed_file);
	_fseeki64(context->compressed_file, (int64_t)0, SEEK_SET);
}

/***********************************************************************************
 * This function calculates each plane size in pixels, taking into account the
 * chroma subsampling used.
 ***********************************************************************************/
static void compute_plane_sizes_in_pixels(yuv2psnrssimContext* context)
{
	int i;

	for (i = 0; i < MAX_NUM_PLANES; i++)
	{
		context->plane_size_in_pixels[i] = (uint64_t)context->width * context->height;
		if (i > 0) //chroma
		{
			switch (context->colorspace)
			{
				case CSP_YUV444:
					// Nothing to do
					break;
				case CSP_YUV422:
					context->plane_size_in_pixels[i] /= 2;
					break;
				case CSP_YUV420:
				default:
					context->plane_size_in_pixels[i] /= 4;
					break;
			}
		}
	}
}

/***********************************************************************************
 * This function allocates memory for each plane. Since internally the program works
 * with 16 bit per pixel planes the size of the allocated memory in bytes will be
 * the size of each plane in pixels multiplied by 2. Also a temporal plane is needed
 * for speed reasons when reading 8 bits or less per pixel files, this time the
 * allocated size in bytes matches the number of pixels.
 ***********************************************************************************/
static void alloc_planes(yuv2psnrssimContext* context)
{
	int i;

	for(i = 0; i < MAX_NUM_PLANES; i++)
	{
		context->original_planes[i] = (uint16_t*)malloc(context->plane_size_in_pixels[i] * 2);
		if(context->original_planes[i] == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Failed to alloc memory.\n\n");
	}

	for(i = 0; i < MAX_NUM_PLANES; i++)
	{
		context->compressed_planes[i] = (uint16_t*)malloc(context->plane_size_in_pixels[i] * 2);
		if(context->compressed_planes[i] == NULL)
			EXIT_ON_ERROR("\n\nERROR -- Failed to alloc memory.\n\n");
	}

	if (context->original_bit_depth <= 8 || context->compressed_bit_depth <= 8)
	{
		for(i = 0; i < MAX_NUM_PLANES; i++)
		{
			context->temp_planes[i] = (uint8_t*)malloc(context->plane_size_in_pixels[i]);
			 if(context->temp_planes[i] == NULL)
				 EXIT_ON_ERROR("\n\nERROR -- Failed to alloc memory.\n\n");
		}
	}
}

/***********************************************************************************
 * This function reads the input files and calls other functions to perform all the
 * different calculations when the files are in YUV format.
 ***********************************************************************************/
static void process_YUV(yuv2psnrssimContext* context)
{
	int i, j;
	uint32_t x;
	uint64_t original_num_frames;
	uint64_t compressed_num_frames;
	uint8_t original_left_shift;
	uint8_t compressed_left_shift;
	int shift;

	context->max_bit_depth = context->original_bit_depth > context->compressed_bit_depth ? context->original_bit_depth : context->compressed_bit_depth;

	get_file_sizes(context);
	compute_plane_sizes_in_pixels(context);

	for (i = 0; i < MAX_NUM_PLANES; i++)
	{
		context->original_frame_size_in_bytes += context->original_bit_depth > 8 ? context->plane_size_in_pixels[i] * 2 : context->plane_size_in_pixels[i];
		context->compressed_frame_size_in_bytes += context->compressed_bit_depth > 8 ? context->plane_size_in_pixels[i] * 2 : context->plane_size_in_pixels[i];
	}
	
	original_num_frames = context->original_file_size_in_bytes / context->original_frame_size_in_bytes;
	compressed_num_frames = context->compressed_file_size_in_bytes / context->compressed_frame_size_in_bytes;

	if (original_num_frames == compressed_num_frames)
		context->num_frames = original_num_frames;
	else
	{
		context->num_frames = original_num_frames>compressed_num_frames ?compressed_num_frames:original_num_frames;
		//EXIT_ON_ERROR("\n\nERROR -- The number of frames of the two files do not match.\n\n");
	}
	if (context->num_frames > MAX_NUM_FRAMES)
		EXIT_ON_ERROR("\n\nERROR -- Files with more than 5000 frames are not supported.\n\n");

	// Alloc memory for each plane
	alloc_planes(context);

	original_left_shift = context->max_bit_depth - context->original_bit_depth;
	compressed_left_shift = context->max_bit_depth - context->compressed_bit_depth;

	for (i = 0; i < context->num_frames; i++)
	{
		for (j = 0; j < MAX_NUM_PLANES; j++)
		{
			if (context->original_bit_depth > 8)
			{
				if (fread(context->original_planes[j], sizeof(uint16_t), context->plane_size_in_pixels[j], context->original_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading original file.\n\n");
				if (original_left_shift != 0)
					for (x = 0; x < context->plane_size_in_pixels[j]; x++)
						context->original_planes[j][x] = context->original_planes[j][x] << original_left_shift;
			}
			else
			{
				if (fread(context->temp_planes[j], sizeof(uint8_t), context->plane_size_in_pixels[j], context->original_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading original file.\n\n");
				for (x = 0; x < context->plane_size_in_pixels[j]; x++)
					context->original_planes[j][x] = ((uint16_t)context->temp_planes[j][x]) << original_left_shift;
			}

			if (context->compressed_bit_depth > 8)
			{
				if (fread(context->compressed_planes[j], sizeof(uint16_t), context->plane_size_in_pixels[j], context->compressed_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading compressed file.\n\n");
				if (compressed_left_shift != 0)
					for (x = 0; x < context->plane_size_in_pixels[j]; x++)
						context->compressed_planes[j][x] = context->compressed_planes[j][x] << compressed_left_shift;
			}
			else
			{
				if (fread(context->temp_planes[j], sizeof(uint8_t), context->plane_size_in_pixels[j], context->compressed_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading compressed file.\n\n");
				for (x = 0; x < context->plane_size_in_pixels[j]; x++)
					context->compressed_planes[j][x] = ((uint16_t)context->temp_planes[j][x]) << compressed_left_shift;
			}
		}

		for (j = 0; j < MAX_NUM_PLANES; j++)
			context->MSE_array[i][j] = compute_plane_MSE(context->original_planes[j], context->compressed_planes[j], context->plane_size_in_pixels[j]);

		//context->MSE_array[i][MAX_NUM_PLANES] = (context->MSE_array[i][0] * 0.8 + context->MSE_array[i][1] * 0.1 + context->MSE_array[i][2] * 0.1);
		context->MSE_array[i][MAX_NUM_PLANES] = (context->MSE_array[i][0]  + context->MSE_array[i][1]  + context->MSE_array[i][2] )/3;

/*			switch (context->colorspace)
			{
				case CSP_YUV444:
					shift = 0;
					break;
				case CSP_YUV422:
					shift = 1;
					break;
				case CSP_YUV420:
				default:
					shift = 2;
					break;
			}	*/	
		for (j = 0; j < MAX_NUM_PLANES; j++)
		  context->SSIM_array[i][j] = compute_plane_SSIM(context->original_planes[j], context->compressed_planes[j], context->width>>((j==0)?0:1), context->height>>((j==0)?0:1), context->max_bit_depth);
		 
		//context->SSIM_array[i][MAX_NUM_PLANES] = (context->SSIM_array[i][0]  + context->SSIM_array[i][1]  + context->SSIM_array[i][2] )/3;
	}
	compute_PSNR(context);
	compute_mean_SSIM(context);
}

/***********************************************************************************
 * This function parses the header present in Y4M files.
 ***********************************************************************************/
static void parse_Y4M_header(yuv2psnrssimContext* context, int isOriginal)
{
	char header[MAX_Y4M_HEADER];
	char* header_end, *tokend, *tokstart;
	int i;

	/* Read header */
	for(i = 0; i < MAX_Y4M_HEADER; i++)
	{
		header[i] = isOriginal ? fgetc(context->original_file) : fgetc(context->compressed_file);
		if( header[i] == '\n' )
		{
			header[i + 1] = 0x20;
			header[i + 2] = '\0';
			break;
		}
	}
	header_end = &header[i + 1];

	// Scan properties
	for(tokstart = &header[strlen(Y4M_SIGNATURE) + 1]; tokstart < header_end; tokstart++)
	{
		if(*tokstart == 0x20)
			continue;
		switch(*tokstart)
		{
			case 'W': // Width. Required.
				if (context->width == 0) // The user didn't enter a resolution.
				{
					context->width = strtol(tokstart + 1, &tokend, 10);
					tokstart = tokend;
				}
				break;
			case 'H': // Height. Required.
				if (context->height == 0) // The user didn't enter a resolution.
				{
					context->height = strtol(tokstart + 1, &tokend, 10);
					tokstart = tokend;
				}
				break;
			case 'C': // Colorspace
				if (!strncmp("420p16", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV420;
					if (isOriginal)
						context->original_bit_depth = 16;
					else
						context->compressed_bit_depth = 16;
				}
				else if (!strncmp("420p10", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV420;
					if (isOriginal)
						context->original_bit_depth = 10;
					else
						context->compressed_bit_depth = 10;
				}
				else if (!strncmp("420p9", tokstart + 1, 5))
				{
					context->colorspace = CSP_YUV420;
					if (isOriginal)
						context->original_bit_depth = 9;
					else
						context->compressed_bit_depth = 9;
				}
				else if (!strncmp("420", tokstart + 1, 3) || !strncmp("420jpeg", tokstart + 1, 7) || !strncmp("420paldv", tokstart + 1, 8) || !strncmp("420mpeg2", tokstart + 1, 8))
					context->colorspace = CSP_YUV420;
				else if (!strncmp("422p16", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV422;
					if (isOriginal)
						context->original_bit_depth = 16;
					else
						context->compressed_bit_depth = 16;
				}
				else if (!strncmp("422p10", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV422;
					if (isOriginal)
						context->original_bit_depth = 10;
					else
						context->compressed_bit_depth = 10;
				}
				else if (!strncmp("422p9", tokstart + 1, 5))
				{
					context->colorspace = CSP_YUV422;
					if (isOriginal)
						context->original_bit_depth = 9;
					else
						context->compressed_bit_depth = 9;
				}
				else if (!strncmp("422", tokstart + 1, 3))
					context->colorspace = CSP_YUV422;
				else if (!strncmp("444p16", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV444;
					if (isOriginal)
						context->original_bit_depth = 16;
					else
						context->compressed_bit_depth = 16;
				}
				else if (!strncmp("444p10", tokstart + 1, 6))
				{
					context->colorspace = CSP_YUV444;
					if (isOriginal)
						context->original_bit_depth = 10;
					else
						context->compressed_bit_depth = 10;
				}
				else if (!strncmp("444p9", tokstart + 1, 5))
				{
					context->colorspace = CSP_YUV444;
					if (isOriginal)
						context->original_bit_depth = 9;
					else
						context->compressed_bit_depth = 9;
				}
				else if (!strncmp("444", tokstart + 1, 3))
					context->colorspace = CSP_YUV444;
				tokstart = strchr(tokstart, 0x20);
				break;
			case 'X': // Vendor extensions. Ignored.
				if(!strncmp("YSCSS=", tokstart + 1, 6))
					tokstart += 6;
				break;		
	}

	if (isOriginal != 0)
		context->Y4M_original_header_size_in_bytes = (uint8_t)strlen(header) - 1;
	else
		context->Y4M_compressed_header_size_in_bytes = (uint8_t)strlen(header) - 1;

	context->Y4M_frame_header_size_in_bytes = (uint8_t)strlen(Y4M_FRAME_SIGNATURE) + 1;
	}
}

/***********************************************************************************
 * This function reads the input files and calls other functions to perform all the
 * different calculations when the files are in Y4M format.
 ***********************************************************************************/
static void process_Y4M(yuv2psnrssimContext* context)
{
	int i, j;
	uint64_t original_num_frames;
	uint64_t compressed_num_frames;
	uint8_t original_left_shift;
	uint8_t compressed_left_shift;
	uint32_t x;
	int shift;
	parse_Y4M_header(context, 1);
	parse_Y4M_header(context, 0);

	context->max_bit_depth = context->original_bit_depth > context->compressed_bit_depth ? context->original_bit_depth : context->compressed_bit_depth;

	get_file_sizes(context);
	compute_plane_sizes_in_pixels(context);

	for (i = 0; i < MAX_NUM_PLANES; i++)
	{
		context->original_frame_size_in_bytes += context->original_bit_depth > 8 ? context->plane_size_in_pixels[i] * 2 : context->plane_size_in_pixels[i];
		context->compressed_frame_size_in_bytes += context->compressed_bit_depth > 8 ? context->plane_size_in_pixels[i] * 2 : context->plane_size_in_pixels[i];
	}
	context->original_frame_size_in_bytes += context->Y4M_frame_header_size_in_bytes;
	context->compressed_frame_size_in_bytes += context->Y4M_frame_header_size_in_bytes;

	original_num_frames = (context->original_file_size_in_bytes - context->Y4M_original_header_size_in_bytes) / context->original_frame_size_in_bytes;
	compressed_num_frames = (context->compressed_file_size_in_bytes - context->Y4M_compressed_header_size_in_bytes) / context->compressed_frame_size_in_bytes;

	if (original_num_frames == compressed_num_frames)
		context->num_frames = original_num_frames;
	else
		EXIT_ON_ERROR("\n\nERROR -- The number of frames of the two files do not match.\n\n");
	if (context->num_frames > MAX_NUM_FRAMES)
		EXIT_ON_ERROR("\n\nERROR -- Files with more than 5000 frames are not supported.\n\n");

	// Alloc memory for each plane
	alloc_planes(context);

	original_left_shift = context->max_bit_depth - context->original_bit_depth;
	compressed_left_shift = context->max_bit_depth - context->compressed_bit_depth;

	for (i = 0; i < context->num_frames; i++)
	{
		if (_fseeki64(context->original_file, context->Y4M_original_header_size_in_bytes + (i * context->original_frame_size_in_bytes) + context->Y4M_frame_header_size_in_bytes, SEEK_SET))
			EXIT_ON_ERROR("\n\nERROR -- Error seeking on original file.\n\n");
		if (_fseeki64(context->compressed_file, context->Y4M_compressed_header_size_in_bytes + (i * context->compressed_frame_size_in_bytes) + context->Y4M_frame_header_size_in_bytes, SEEK_SET))
			EXIT_ON_ERROR("\n\nERROR -- Error seeking on compressed file.\n\n");
		for (j = 0; j < MAX_NUM_PLANES; j++)
		{
			if (context->original_bit_depth > 8)
			{
				if (fread(context->original_planes[j], sizeof(uint16_t), context->plane_size_in_pixels[j], context->original_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading original file.\n\n");
				if (original_left_shift != 0)
					for (x = 0; x < context->plane_size_in_pixels[j]; x++)
						context->original_planes[j][x] = context->original_planes[j][x] << original_left_shift;
			}
			else
			{
				if (fread(context->temp_planes[j], sizeof(uint8_t), context->plane_size_in_pixels[j], context->original_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading original file.\n\n");
				for (x = 0; x < context->plane_size_in_pixels[j]; x++)
					context->original_planes[j][x] = ((uint16_t)context->temp_planes[j][x]) << original_left_shift;
			}

			if (context->compressed_bit_depth > 8)
			{
				if (fread(context->compressed_planes[j], sizeof(uint16_t), context->plane_size_in_pixels[j], context->compressed_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading compressed file.\n\n");
				if (compressed_left_shift != 0)
					for (x = 0; x < context->plane_size_in_pixels[j]; x++)
						context->compressed_planes[j][x] = context->compressed_planes[j][x] << compressed_left_shift;
			}
			else
			{
				if (fread(context->temp_planes[j], sizeof(uint8_t), context->plane_size_in_pixels[j], context->compressed_file) != context->plane_size_in_pixels[j])
					EXIT_ON_ERROR("\n\nERROR -- Error reading compressed file.\n\n");
				for (x = 0; x < context->plane_size_in_pixels[j]; x++)
					context->compressed_planes[j][x] = ((uint16_t)context->temp_planes[j][x]) << compressed_left_shift;
			}
		}

		for (j = 0; j < MAX_NUM_PLANES; j++)
			context->MSE_array[i][j] = compute_plane_MSE(context->original_planes[j], context->compressed_planes[j], context->plane_size_in_pixels[j]);
		//context->MSE_array[i][MAX_NUM_PLANES] = (context->MSE_array[i][0] * 0.8 + context->MSE_array[i][1] * 0.1 + context->MSE_array[i][2] * 0.1);
		context->MSE_array[i][MAX_NUM_PLANES] = (context->MSE_array[i][0]  + context->MSE_array[i][1]  + context->MSE_array[i][2] )/3;

		
			switch (context->colorspace)
			{
				case CSP_YUV444:
					shift = 0;
					break;
				case CSP_YUV422:
					shift = 1;
					break;
				case CSP_YUV420:
				default:
					shift = 2;
					break;
			}
		for (j = 0; j < MAX_NUM_PLANES; j++)
		  context->SSIM_array[i][j] = compute_plane_SSIM(context->original_planes[j], context->compressed_planes[j], context->width>>shift, context->height>>shift, context->max_bit_depth);
		 
		//context->SSIM_array[i] = compute_plane_SSIM(context->original_planes[0], context->compressed_planes[0], context->width, context->height, context->max_bit_depth);
	}
	compute_PSNR(context);
	compute_mean_SSIM(context);
}

/***********************************************************************************
 * This function prints the results of the different calculations to screen.
 ***********************************************************************************/
static void print_results(const yuv2psnrssimContext* context)
{
	int i;

	printf_s("Original file: %s\nCompressed file: %s\n\n", context->original_file_name, context->compressed_file_name);
	context->file_format == YUV ? printf_s("File format: uncompressed YUV\n") : 	printf_s("File format: YUV4MPEG2 (uncompressed YUV)\n");
	printf_s("Resolution: %ux%u pixels\n", context->width, context->height);
	switch (context->colorspace)
	{
		default:
		case 1:
			printf_s("Colorspace: YUV 4:2:0\n");
			break;
		case 2:
			printf_s("Colorspace: YUV 4:2:2\n");
			break;
		case 3:
			printf_s("Colorspace: YUV 4:4:4\n");
			break;
	}
	printf_s("Original file bits per sample: %hu bits\n", context->original_bit_depth);
	printf_s("Compressed file bits per sample: %hu bits\n", context->compressed_bit_depth);
	printf_s("Number of frames: %llu\n", context->num_frames);
	if (context->file_format == YUV)
	{
		printf_s("Original file frame size: %llu bytes\n", context->original_frame_size_in_bytes);
		printf_s("Compressed file frame size: %llu bytes\n\n", context->compressed_frame_size_in_bytes);
	}
	else
	{
		printf_s("Original file frame size: %llu bytes\n", context->original_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
		printf_s("Compressed file frame size: %llu bytes\n\n", context->compressed_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
	}

	for (i = 0; i < context->num_frames; i++)
	{
		printf_s("Frame %d, PSNR Y: %Lg dB\n", i + 1, context->PSNR_array[i][0]);
		printf_s("Frame %d, PSNR Cb: %Lg dB\n", i + 1, context->PSNR_array[i][1]);
		printf_s("Frame %d, PSNR Cr: %Lg dB\n", i + 1, context->PSNR_array[i][2]);
		printf_s("Frame %d, combined PSNR: %Lg dB\n", i + 1, context->PSNR_array[i][3]);
		printf_s("Frame %d, SSIM Y: %Lg\n\n", i + 1, context->SSIM_array[i][0]);
	}

	printf_s("\nPSNR mean Y: %Lg dB\n", context->PSNR_meanY);
	printf_s("PSNR mean Cb: %Lg dB\n", context->PSNR_meanCb);
	printf_s("PSNR mean Cr: %Lg dB\n", context->PSNR_meanCr);
	printf_s("Combined PSNR: %Lg dB\n", context->PSNR_combined);
	printf_s("SSIM mean Y: %Lg\n\n", context->SSIM_meanY);
	printf_s("SSIM mean Cb: %Lg\n\n", context->SSIM_meanCb);
	printf_s("SSIM mean Cr: %Lg\n\n", context->SSIM_meanCr);
	printf_s("Combined SSIM: %Lg\n\n", context->SSIM_combined);

	printf_s("Processing took %lf seconds.\n\n", context->time);
}

/***********************************************************************************
 * This function writes the results of the different calculations to the file
 * specified in csv_file. The format of the file varies depending on csv_decimal_separator.
 ***********************************************************************************/
static void write_csv(const yuv2psnrssimContext* context)
{
	int i, j;
	char PSNR_meanY[100], PSNR_meanCb[100], PSNR_meanCr[100], PSNR_combined[100], SSIM_meanY[100], SSIM_meanCb[100], SSIM_meanCr[100], SSIM_combined[100];

	if (context->csv_decimal_separator == COMMA)
	{
		fputs("\"yuv2psnrssim csv output file\";;;;;\r\n;;;;;\r\n", context->csv_file);

		fprintf(context->csv_file, "\"Original file:\";\"%s\";;;;\r\n", context->original_file_name);
		fprintf(context->csv_file, "\"Compressed file:\";\"%s\";;;;\r\n;;;;;\r\n", context->compressed_file_name);

		context->file_format == YUV ? fputs("\"File format:\";\"uncompressed YUV\";;;;\r\n", context->csv_file) : fputs("\"File format:\";\"YUV4MPEG2 (uncompressed YUV)\";;;;\r\n", context->csv_file);
		fprintf(context->csv_file, "\"Resolution:\";\"%ux%u pixels\";;;;\r\n", context->width, context->height);
		switch (context->colorspace)
		{
			default:
			case 1:
				fputs("\"Colorspace:\";\"YUV 4:2:0\";;;;\r\n", context->csv_file);
				break;
			case 2:
				fputs("\"Colorspace:\";\"YUV 4:2:2\";;;;\r\n", context->csv_file);
				break;
			case 3:
				fputs("\"Colorspace:\";\"YUV 4:4:4\";;;;\r\n", context->csv_file);
				break;
		}
		fprintf(context->csv_file, "\"Original file bits per sample:\";\"%hu bits\";;;;\r\n", context->original_bit_depth);
		fprintf(context->csv_file, "\"Compressed file bits per sample:\";\"%hu bits\";;;;\r\n", context->compressed_bit_depth);
		fprintf(context->csv_file, "\"Number of frames:\";\"%llu\";;;;\r\n", context->num_frames);
		if (context->file_format == YUV)
		{
			fprintf(context->csv_file, "\"Original file frame size:\";\"%llu bytes\";;;;\r\n", context->original_frame_size_in_bytes);
			fprintf(context->csv_file, "\"Compressed file frame size:\";\"%llu bytes\";;;;\r\n;;;;;\r\n", context->compressed_frame_size_in_bytes);
		}
		else
		{
			fprintf(context->csv_file, "\"Original file frame size:\";\"%llu bytes\";;;;\r\n", context->original_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
			fprintf(context->csv_file, "\"Compressed file frame size:\";\"%llu bytes\";;;;\r\n;;;;;\r\n", context->compressed_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
		}

		fputs("\"\";\"PSNR Y\";\"PSNR Cb\";\"PSNR Cr\";\"Combined PSNR\";\"SSIM Y\"\r\n", context->csv_file);

		for (i = 0; i < context->num_frames; i++)
		{
			char PSNR_Y[100], PSNR_Cb[100], PSNR_Cr[100], PSNR_combined[100], SSIM_Y[100];
			sprintf_s(PSNR_Y, 100, "%Lg", context->PSNR_array[i][0]);				
			sprintf_s(PSNR_Cb, 100, "%Lg", context->PSNR_array[i][1]);
			sprintf_s(PSNR_Cr, 100, "%Lg", context->PSNR_array[i][2]);
			sprintf_s(PSNR_combined, 100, "%Lg", context->PSNR_array[i][3]);
			sprintf_s(SSIM_Y, 100, "%Lg", context->SSIM_array[i][0]);
			for(j = 0; j < 50; j++)
			{
				if(PSNR_Y[j] == '.') PSNR_Y[j] = ',';
				if(PSNR_Cb[j] == '.') PSNR_Cb[j] = ',';
				if(PSNR_Cr[j] == '.') PSNR_Cr[j] = ',';
				if(PSNR_combined[j] == '.') PSNR_combined[j] = ',';
				if(SSIM_Y[j] == '.') SSIM_Y[j] = ',';
			}
			fprintf(context->csv_file, "\"Frame %d\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\"\r\n",
					i + 1, PSNR_Y, PSNR_Cb, PSNR_Cr, PSNR_combined, SSIM_Y);
		}

		sprintf_s(PSNR_meanY, 100, "%Lg", context->PSNR_meanY);
		sprintf_s(PSNR_meanCb, 100, "%Lg", context->PSNR_meanCb);
		sprintf_s(PSNR_meanCr, 100, "%Lg", context->PSNR_meanCr);
		sprintf_s(PSNR_combined, 100, "%Lg", context->PSNR_combined);
		sprintf_s(SSIM_meanY, 100, "%Lg", context->SSIM_meanY);
		sprintf_s(SSIM_meanCb, 100, "%Lg", context->SSIM_meanCb);
		sprintf_s(SSIM_meanCr, 100, "%Lg", context->SSIM_meanCr);
		sprintf_s(SSIM_combined, 100, "%Lg", context->SSIM_combined);
		for(i = 0; i < 50; i++)
		{
			if(PSNR_meanY[i] == '.') PSNR_meanY[i] = ',';
			if(PSNR_meanCb[i] == '.') PSNR_meanCb[i] = ',';
			if(PSNR_meanCr[i] == '.') PSNR_meanCr[i] = ',';
			if(PSNR_combined[i] == '.') PSNR_combined[i] = ',';
			if(SSIM_meanY[i] == '.') SSIM_meanY[i] = ',';
			if(SSIM_meanCb[i] == '.') SSIM_meanCb[i] = ',';
			if(SSIM_meanCr[i] == '.') SSIM_meanCr[i] = ',';
			if(SSIM_combined[i] == '.') SSIM_combined[i] = ',';
		}
		fprintf(context->csv_file, ";;;;;\r\n\"PSNR mean Y:\";\"%s\";;;;\r\n", PSNR_meanY);
		fprintf(context->csv_file, "\"PSNR mean Cb:\";\"%s\";;;;\r\n", PSNR_meanCb);
		fprintf(context->csv_file, "\"PSNR mean Cr:\";\"%s\";;;;\r\n", PSNR_meanCr);
		fprintf(context->csv_file, "\"Combined PSNR:\";\"%s\";;;;\r\n", PSNR_combined);
		fprintf(context->csv_file, "\"SSIM mean Y:\";\"%s\";;;;", SSIM_meanY);
		fprintf(context->csv_file, "\"SSIM mean Cb:\";\"%s\";;;;", SSIM_meanCb);
		fprintf(context->csv_file, "\"SSIM mean Cr:\";\"%s\";;;;", SSIM_meanCr);
		fprintf(context->csv_file, "\"Combined SSIM:\";\"%s\";;;;", SSIM_combined);
	}
	else // FIXME: Dot as decimal separator, NOT TESTED
	{
		fputs("\"yuv2psnrssim csv output file\",,,,,\r\n,,,,,\r\n", context->csv_file);

		fprintf(context->csv_file, "\"Original file:\",\"%s\",,,,\r\n", context->original_file_name);
		fprintf(context->csv_file, "\"Compressed file:\",\"%s\",,,,\r\n,,,,,\r\n", context->compressed_file_name);

		context->file_format == YUV ? fputs("\"File format:\",\"uncompressed YUV\",,,,\r\n", context->csv_file) : fputs("\"File format:\",\"YUV4MPEG2 (uncompressed YUV)\",,,,\r\n", context->csv_file);
		fprintf(context->csv_file, "\"Resolution:\",\"%ux%u pixels\",,,,\r\n", context->width, context->height);
		switch (context->colorspace)
		{
			default:
			case 1:
				fputs("\"Colorspace:\",\"YUV 4:2:0\",,,,\r\n", context->csv_file);
				break;
			case 2:
				fputs("\"Colorspace:\",\"YUV 4:2:2\",,,,\r\n", context->csv_file);
				break;
			case 3:
				fputs("\"Colorspace:\",\"YUV 4:4:4\",,,,\r\n", context->csv_file);
				break;
		}
		fprintf(context->csv_file, "\"Original file bits per sample:\",\"%hu bits\",,,,\r\n", context->original_bit_depth);
		fprintf(context->csv_file, "\"Compressed file bits per sample:\",\"%hu bits\",,,,\r\n", context->compressed_bit_depth);
		fprintf(context->csv_file, "\"Number of frames:\",\"%llu\",,,,\r\n", context->num_frames);
		if (context->file_format == YUV)
		{
			fprintf(context->csv_file, "\"Original file frame size:\",\"%llu bytes\",,,,\r\n", context->original_frame_size_in_bytes);
			fprintf(context->csv_file, "\"Compressed file frame size:\",\"%llu bytes\",,,,\r\n,,,,,\r\n", context->compressed_frame_size_in_bytes);
		}
		else
		{
			fprintf(context->csv_file, "\"Original file frame size:\",\"%llu bytes\",,,,\r\n", context->original_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
			fprintf(context->csv_file, "\"Compressed file frame size:\",\"%llu bytes\",,,,\r\n,,,,,\r\n", context->compressed_frame_size_in_bytes - context->Y4M_frame_header_size_in_bytes);
		}

		fputs("\"\",\"PSNR Y\",\"PSNR Cb\",\"PSNR Cr\",\"Combined PSNR\",\"SSIM Y\"\r\n", context->csv_file);

		for (i = 0; i < context->num_frames; i++)
		{
			fprintf(context->csv_file, "\"Frame %d\",", i + 1);
			fprintf(context->csv_file, "\"%Lg\",", context->PSNR_array[i][0]);
			fprintf(context->csv_file, "\"%Lg\",", context->PSNR_array[i][1]);
			fprintf(context->csv_file, "\"%Lg\",", context->PSNR_array[i][2]);
			fprintf(context->csv_file, "\"%Lg\",", context->PSNR_array[i][3]);
			fprintf(context->csv_file, "\"%Lg\"\r\n", context->SSIM_array[i][0]);
		}

		fprintf(context->csv_file, ",,,,,\r\n\"PSNR mean Y:\",\"%Lg\",,,,\r\n", context->PSNR_meanY);
		fprintf(context->csv_file, "\"PSNR mean Cb:\",\"%Lg\",,,,\r\n", context->PSNR_meanCb);
		fprintf(context->csv_file, "\"PSNR mean Cr:\",\"%Lg\",,,,\r\n", context->PSNR_meanCr);
		fprintf(context->csv_file, "\"Combined PSNR:\",\"%Lg\",,,,\r\n", context->PSNR_combined);
		fprintf(context->csv_file, "\"SSIM mean Y:\",\"%Lg\",,,,\r\n", context->SSIM_meanY);
		fprintf(context->csv_file, "\"SSIM mean Cb:\",\"%Lg\",,,,\r\n", context->SSIM_meanCb);
		fprintf(context->csv_file, "\"SSIM mean Cr:\",\"%Lg\",,,,\r\n", context->SSIM_meanCr);
		fprintf(context->csv_file, "\"Combined SSIM:\",\"%Lg\",,,,", context->SSIM_combined);
	}
}

/***********************************************************************************
 * This function deallocates the memory previously allocated for each plane.
 ***********************************************************************************/
static void free_planes(yuv2psnrssimContext* context)
{
	int i;

	for(i = 0; i < MAX_NUM_PLANES; i++)
	{
		free(context->original_planes[i]);
		context->original_planes[i] = NULL;
	}

	for(i = 0; i < MAX_NUM_PLANES; i++)
	{
		free(context->compressed_planes[i]);
		context->compressed_planes[i] = NULL;
	}

	for(i = 0; i < MAX_NUM_PLANES; i++)
	{
		free(context->temp_planes[i]);
		context->temp_planes[i] = NULL;
	}
}

/***********************************************************************************
 * This function deallocates used memory and closes the input files.
 ***********************************************************************************/
static void clean(yuv2psnrssimContext* context)
{
	// Free memory
	free_planes(context);

	// Close the files
	fclose(context->original_file);
	fclose(context->compressed_file);
	if (context->csv_file_name != NULL)
		fclose(context->csv_file);
}

/***********************************************************************************
 * Main function.
 ***********************************************************************************/
int main(int argc, char* argv[])
{
	double startTime = (double)clock()/CLOCKS_PER_SEC;
	yuv2psnrssimContext context;

	initialize_context(&context);

	parse(argc, argv, &context);

	open_files(&context);

	printf_s("\n\nyuv2psnrssim is processing results, this may take a while...\n\n");
	switch (context.file_format)
	{
		case YUV:
			process_YUV(&context);
			break;
		case Y4M:
			process_Y4M(&context);
			break;
	}

	context.time = (double)clock()/CLOCKS_PER_SEC - startTime;

	if (context.print_results)
		print_results(&context);
	if (context.csv_file_name)
		write_csv(&context);

	clean(&context);

#ifdef _DEBUG
	system("PAUSE");
#endif

	return 0;
}