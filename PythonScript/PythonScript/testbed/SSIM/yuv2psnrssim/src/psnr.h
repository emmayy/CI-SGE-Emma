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

#ifndef _PSNR_H
#define _PSNR_H

#include "yuv2psnrssim.h"

/***********************************************************************************
 * This function calculates the Mean Squared Error of a plane.
 ***********************************************************************************/
double compute_plane_MSE(uint16_t* original_plane, uint16_t* compressed_plane, const uint32_t plane_size_in_pixels);

/***********************************************************************************
 * This function calculates the following PSNR values:
 * - For each frame, PSNR Y, PSNR Cb and PSNR Cr.
 * - For each frame, the combined value of the PSNR Y, PSNR Cb and PSNR Cr values.
 * - The average of all PSNR Y, PSNR Cb and PSNR Cr values to form the PSNR mean Y,
 *   PSNR mean Cb and PSNR mean Cr values.
 * - The combined PSNR of the entire video sequence.
 * Maximum PSNR for a given plane is 100 dB.
 ***********************************************************************************/
void compute_PSNR(yuv2psnrssimContext* context);

#endif