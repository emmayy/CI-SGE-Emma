;/***********************************************************************
 ;*	Copyright (c) 2004-2012 WebEx Communications, Inc.
 ;*  All rights reserved	
 ;*
 ;*	  WelsCPU.h
 ;*
 ;*  Author
 ;*      Derrick Jin (xingji@cisco.com)
 ;*
 ;*  Abstract
 ;*		verify cpuid feature support and cpuid detection
 ;*
 ;*  History
 ;*      6/4/2012	Created
 ;*
 ;*************************************************************************/

#ifndef _WELS_CPU_H_
#define _WELS_CPU_H_

#include "WelsCommonType.h"
#include "WelsOSAbstract.h"

namespace WelsUtilLib{

/*
 *	WELS CPU feature flags
 */ 
#define WELS_CPU_MMX        0x00000001    /* mmx */
#define WELS_CPU_MMXEXT     0x00000002    /* mmx-ext*/
#define WELS_CPU_SSE        0x00000004    /* sse */
#define WELS_CPU_SSE2       0x00000008    /* sse 2 */
#define WELS_CPU_SSE3       0x00000010    /* sse 3 */
#define WELS_CPU_SSE41      0x00000020    /* sse 4.1 */
#define WELS_CPU_3DNOW      0x00000040    /* 3dnow! */
#define WELS_CPU_3DNOWEXT   0x00000080    /* 3dnow! ext */
#define WELS_CPU_ALTIVEC    0x00000100    /* altivec */
#define WELS_CPU_SSSE3      0x00000200    /* ssse3 */
#define WELS_CPU_SSE42      0x00000400    /* sse 4.2 */

/* CPU features application extensive */
#define WELS_CPU_AVX		0x00000800	/* Advanced Vector eXtentions */
#define WELS_CPU_FPU		0x00001000	/* x87-FPU on chip */
#define WELS_CPU_HTT		0x00002000	/* Hyper-Threading Technology (HTT), Multi-threading enabled feature: 
										   physical processor package is capable of supporting more than one logic processor
										*/
#define WELS_CPU_CMOV		0x00004000	/* Conditional Move Instructions,
										   also if x87-FPU is present at indicated by the CPUID.FPU feature bit, then FCOMI and FCMOV are supported
										*/
#define WELS_CPU_MOVBE		0x00008000	/* MOVBE instruction */
#define WELS_CPU_AES		0x00010000	/* AES instruction extensions */

#define WELS_CPU_CACHELINE_16    0x10000000    /* CacheLine Size 16 */
#define WELS_CPU_CACHELINE_32    0x20000000    /* CacheLine Size 32 */
#define WELS_CPU_CACHELINE_64    0x40000000    /* CacheLine Size 64 */
#define WELS_CPU_CACHELINE_128   0x80000000    /* CacheLine Size 128 */

	/* For the android OS */
#define WELS_CPU_ARMv7      0x000001    /* ARMv7 */
#define WELS_CPU_VFPv3      0x000002    /* VFPv3 */
#define WELS_CPU_NEON       0x000004    /* NEON */

//namespace WelsUtilLib{
   
#if defined(__cplusplus)
    extern "C" {
#endif//__cplusplus

#ifdef HAVE_MMX

 int32_t  wels_cpuid_verify();
 
 void wels_cpu_cpuid( uint32_t index, uint32_t *feature_a, uint32_t *feature_b, uint32_t *feature_c, uint32_t *feature_d );
 
#endif

uint32_t wels_cpu_feature_detect( uint32_t *number_of_logic_processors );

#if defined(__cplusplus)
    }
#endif//__cplusplus

}//namespace WelsHEVCEncoder

#endif

