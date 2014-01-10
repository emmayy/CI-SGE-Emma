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

#ifndef _SSIM_H
#define _SSIM_H

#include "yuv2psnrssim.h"

/***********************************************************************************
 * This function computes the SSIM value of the luminance plane of a given frame.
 * It uses a 8x8 moving window with the starting location (top-left pixel) of each
 * window located on the intersections of an imaginary 4x4 pixel grid. Such arrangement
 * allows the windows to overlap block boundaries to penalize blocking artifacts.
 * The mean of all the possible windows is the SSIM value for that plane.
 * The code takes inspiration from the SSIM implementations in:
 * libvp8 project: http://git.chromium.org/gitweb/?p=webm/libvpx.git;f=vp8/encoder/ssim.c;hb=HEAD
 * x264 project: http://git.videolan.org/?p=x264.git;f=common/pixel.c;hb=d9d2288cabcfd1a90f29f2f11c8cce5450a08ffa#l583
 ***********************************************************************************/
double compute_plane_SSIM(uint16_t* original_plane, uint16_t* compressed_plane, const uint32_t width, const uint32_t height, const uint8_t max_bit_depth);

/***********************************************************************************
 * This function calculates the average of all SSIM Y frame values.
 ***********************************************************************************/
void compute_mean_SSIM(yuv2psnrssimContext* context);

#endif