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

#include "ssim.h"

double compute_plane_SSIM(uint16_t* original_plane, uint16_t* compressed_plane, const uint32_t width, const uint32_t height, const uint8_t max_bit_depth)
{
	uint32_t i, j, k, l;
	uint64_t num_8x8_windows = 0;
	uint64_t sum_X = 0, sum_Y = 0, sum_squared_X = 0, sum_squared_Y = 0, sum_X_mul_Y = 0, pixel_X = 0, pixel_Y = 0;
	double average_X = 0, average_Y = 0, variance_X = 0, variance_Y = 0, covariance_XY = 0;
	const double C1 = (0.01*((1 << max_bit_depth) - 1)) * (0.01*((1 << max_bit_depth) - 1));
	const double C2 = (0.03*((1 << max_bit_depth) - 1)) * (0.03*((1 << max_bit_depth) - 1));
	double ssim_total = 0;

	// Sample point start with each 4x4 location
	for(i = 0; i <= height - 8; i += 4, original_plane += width * 4, compressed_plane += width * 4)
	{
		for(j = 0; j <= width - 8; j += 4)
		{
			// 8x8 window SSIM calculation
			sum_X = sum_Y = sum_squared_X = sum_squared_Y = sum_X_mul_Y = pixel_X = pixel_Y = 0;
			average_X = average_Y = variance_X = variance_Y = covariance_XY = 0;
			for (k = 0; k < 8; k++)
			{
				for (l = 0; l < 8; l++)
				{
					pixel_X = (uint64_t)original_plane[j + k * width + l];
					pixel_Y = (uint64_t)compressed_plane[j + k * width + l];
					sum_X += pixel_X;
					sum_Y += pixel_Y;
					sum_squared_X += pixel_X * pixel_X;
					sum_squared_Y += pixel_Y * pixel_Y;
					sum_X_mul_Y += pixel_X * pixel_Y;
				}
			}
			average_X = (double)sum_X / 64;
			average_Y = (double)sum_Y / 64;
			variance_X = ((double)sum_squared_X / 64) - (average_X * average_X);
			variance_Y = ((double)sum_squared_Y / 64) - (average_Y * average_Y);
			covariance_XY = ((double)sum_X_mul_Y / 64) - (average_X * average_Y);

			ssim_total += ((2 * average_X * average_Y + C1) * (2 * covariance_XY + C2))
						/ (((average_X * average_X) + (average_Y * average_Y) + C1) * (variance_X + variance_Y + C2));

			num_8x8_windows++;
		}
	}
	return ssim_total /= num_8x8_windows;
}

void compute_mean_SSIM(yuv2psnrssimContext* context)
{
	double sum_SSIM = 0;
	double sum_SSIM_Cb = 0;
	double sum_SSIM_Cr = 0;
	double sum_SSIM_combined = 0;
	uint32_t i;

	for (i = 0; i < context->num_frames; i++)
	{
		   sum_SSIM += context->SSIM_array[i][0];
		   sum_SSIM_Cb += context->SSIM_array[i][1];
		   sum_SSIM_Cr += context->SSIM_array[i][2];
	}
	context->SSIM_meanY = sum_SSIM / context->num_frames;
	context->SSIM_meanCb = sum_SSIM_Cb / context->num_frames;
	context->SSIM_meanCr = sum_SSIM_Cr / context->num_frames;

	context->SSIM_combined = (context->SSIM_meanY + context->SSIM_meanCb +context->SSIM_meanCr)/3;
}