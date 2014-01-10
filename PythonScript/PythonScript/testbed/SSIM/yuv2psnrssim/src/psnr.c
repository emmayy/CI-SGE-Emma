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

#include "psnr.h"
#include <math.h>

double compute_plane_MSE(uint16_t* original_plane, uint16_t* compressed_plane, const uint32_t plane_size_in_pixels)
{
	double difference = 0;
	double sum_MSE = 0;
	uint32_t i;

	for(i = 0; i < plane_size_in_pixels; i++)
	{
		difference = original_plane[i] - compressed_plane[i];
		sum_MSE += difference * difference;
	}

	return sum_MSE /= plane_size_in_pixels;
}

void compute_PSNR(yuv2psnrssimContext* context)
{
	double sum_MSE_Y = 0;
	double sum_MSE_Cb = 0;
	double sum_MSE_Cr = 0;
	double sum_MSE_combined = 0;
	double PSNR_temp = 0;
	uint32_t pixel_max = (1 << context->max_bit_depth) - 1;
	uint32_t i, j;

	for (i = 0; i < context->num_frames; i++)
	{
		for(j = 0; j < MAX_NUM_PLANES; j++)
		{
			PSNR_temp = 10 * log10(pixel_max * pixel_max / context->MSE_array[i][j]);
			context->PSNR_array[i][j] = PSNR_temp > 100 ? 100 : PSNR_temp;
		}

		sum_MSE_Y += context->MSE_array[i][0];
		sum_MSE_Cb += context->MSE_array[i][1];
		sum_MSE_Cr += context->MSE_array[i][2];
		sum_MSE_combined += context->MSE_array[i][3];

		PSNR_temp = 10 * log10(pixel_max * pixel_max / context->MSE_array[i][MAX_NUM_PLANES]);
		context->PSNR_array[i][MAX_NUM_PLANES] = PSNR_temp > 100 ? 100 : PSNR_temp;
	}

	PSNR_temp = 10 * log10(pixel_max * pixel_max / (sum_MSE_Y / context->num_frames));
	context->PSNR_meanY = PSNR_temp > 100 ? 100 : PSNR_temp;

	PSNR_temp = 10 * log10(pixel_max * pixel_max / (sum_MSE_Cb / context->num_frames));
	context->PSNR_meanCb = PSNR_temp > 100 ? 100 : PSNR_temp;

	PSNR_temp = 10 * log10(pixel_max * pixel_max / (sum_MSE_Cr / context->num_frames));
	context->PSNR_meanCr = PSNR_temp > 100 ? 100 : PSNR_temp;

	PSNR_temp = 10 * log10(pixel_max * pixel_max / (sum_MSE_combined / context->num_frames));
	context->PSNR_combined = PSNR_temp > 100 ? 100 : PSNR_temp;
}