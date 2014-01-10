;/***********************************************************************
 ;*	Copyright (c) 2004-2012 WebEx Communications, Inc.
 ;*  All rights reserved	
 ;*
 ;*	  WelsCPU.cpp
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


#include <string>

#ifdef ANDROID_NDK
#include <cpu-features.h>
#endif
#ifdef APPLE_IOS
#include <sys/utsname.h>
#endif

#include "WelsCPU.h"
#include "WelsUtility.h"

namespace WelsUtilLib{

#define    CPU_Vender_AMD    std::string("AuthenticAMD")
#define    CPU_Vender_INTEL   std::string("GenuineIntel")
#define    CPU_Vender_CYRIX   std::string("CyrixInstead")

#ifdef HAVE_MMX

uint32_t wels_cpu_feature_detect( uint32_t *number_of_logic_processors )
{
    uint32_t cpu = 0;	
    uint32_t feature_a = 0, feature_b = 0, feature_c = 0, feature_d = 0;
    int32_t  cache_line_size = 0;
    int8_t   vender_name[16] = { 0 };	

    if( !wels_cpuid_verify() )
    {
        /* cpuid is not supported in cpu */
        return 0;
    }

    wels_cpu_cpuid( 0, &feature_a, (uint32_t*)&vender_name[0],(uint32_t*)&vender_name[8],(uint32_t*)&vender_name[4] );
    if( feature_a == 0 )
    {
        /* maximum input value for basic cpuid information */
        return 0;
    }
    
    std::string strVenderName((char*)(vender_name));
    wels_cpu_cpuid( 1, &feature_a, &feature_b, &feature_c, &feature_d );
    if( (feature_d & 0x00800000) == 0 )
    {
        /* Basic MMX technology is not support in cpu, mean nothing for us so return here */
        return 0;
    }

    cpu = WELS_CPU_MMX;
    if( feature_d & 0x02000000 )
    {
        /* SSE technology is identical to AMD MMX extensions */
        cpu |= WELS_CPU_MMXEXT|WELS_CPU_SSE;
    }
    if( feature_d & 0x04000000 )
    {
        /* SSE2 support here */
        cpu |= WELS_CPU_SSE2;
    }
    if ( feature_d & 0x00000001 )
    {
        /* x87 FPU on-chip checking */
        cpu |= WELS_CPU_FPU;
    }
    if ( feature_d & 0x00008000 )
    {
        /* CMOV instruction checking */
        cpu |= WELS_CPU_CMOV;
    }
  
    if (!strVenderName.compare(CPU_Vender_INTEL))
   // if ( !strcmp((const char*)vender_name,CPU_Vender_INTEL) )	// confirmed_safe_unsafe_usage
    {
        if ( feature_d & 0x10000000 )
        {
            /* Multi-Threading checking: contains of multiple logic processors */
            cpu |= WELS_CPU_HTT;
        }
    }	

    if( feature_c & 0x00000001 ){
        /* SSE3 support here */
        cpu |= WELS_CPU_SSE3;
    }
    if( feature_c & 0x00000200 ){
        /* SSSE3 support here */
        cpu |= WELS_CPU_SSSE3;
    }
    if( feature_c & 0x00080000 ){
        /* SSE4.1 support here, 45nm Penryn processor */
        cpu |= WELS_CPU_SSE41; 
    }
    if( feature_c & 0x00100000 ){
        /* SSE4.2 support here, next generation Nehalem processor */
        cpu |= WELS_CPU_SSE42;
    }
    if ( feature_c & 0x10000000 )
    {
        /* AVX checking */
        cpu |= WELS_CPU_AVX;
    }
    if ( feature_c & 0x02000000 )
    {
        /* AES checking */
        cpu |= WELS_CPU_AES;
    }
    if ( feature_c & 0x00400000 )
    {
        /* MOVBE checking */
        cpu |= WELS_CPU_MOVBE;
    }

    if ( number_of_logic_processors != NULL )
    {
        // HTT enabled on chip
        *number_of_logic_processors = (feature_b & 0x00ff0000) >> 16; // feature bits: 23-16 on returned EBX		
    }	

    *number_of_logic_processors = WELS_MAX(1,(*number_of_logic_processors));

    wels_cpu_cpuid( 0x80000000, &feature_a, &feature_b, &feature_c, &feature_d );

    if( (!strVenderName.compare(CPU_Vender_AMD)) && (feature_a>=0x80000001) ){
   // if( (!strcmp((const char*)vender_name,CPU_Vender_AMD)) && (feature_a>=0x80000001) ){	// confirmed_safe_unsafe_usage
        wels_cpu_cpuid(0x80000001, &feature_a, &feature_b, &feature_c, &feature_d );
        if( feature_d&0x00400000 ){
            cpu |= WELS_CPU_MMXEXT;
        }
        if( feature_d&0x80000000 ){
            cpu |= WELS_CPU_3DNOW;
        }
    }
      if (!strVenderName.compare(CPU_Vender_INTEL)){
  //  if( !strcmp((const char*)vender_name,CPU_Vender_INTEL) ){	// confirmed_safe_unsafe_usage
        int32_t  family, model;

        wels_cpu_cpuid(1, &feature_a, &feature_b, &feature_c, &feature_d);
        family = ((feature_a>>8)&0xf) + ((feature_a>>20)&0xff);
        model  = ((feature_a>>4)&0xf) + ((feature_a>>12)&0xf0);

        if( (family==6) && (model==9 || model==13 || model==14) ){
            cpu &= ~(WELS_CPU_SSE2|WELS_CPU_SSE3);
        }
    }

    // get cache line size
    if( (!strVenderName.compare(CPU_Vender_INTEL)) || !(strVenderName.compare(CPU_Vender_CYRIX)) ){	// confirmed_safe_unsafe_usage
        wels_cpu_cpuid(1, &feature_a, &feature_b, &feature_c, &feature_d);

        cache_line_size = (feature_b&0xff00)>>5;	// ((clflush_line_size >> 8) << 3), CLFLUSH_line_size * 8 = cache_line_size_in_byte

        if( cache_line_size == 128 ){
            cpu |= WELS_CPU_CACHELINE_128;
        }
        else if( cache_line_size == 64 ){
            cpu |= WELS_CPU_CACHELINE_64;
        }
        else if( cache_line_size == 32 ){
            cpu |= WELS_CPU_CACHELINE_32;
        }
        else if( cache_line_size == 16 ){
            cpu |= WELS_CPU_CACHELINE_16;
        }
    }

    return cpu;
}

#elif defined(ANDROID_NDK)
uint32_t wels_cpu_feature_detect()
{
	uint32_t         cpu = 0;
    AndroidCpuFamily cpuFamily = ANDROID_CPU_FAMILY_UNKNOWN;
    uint64_t         features = 0;

    cpuFamily = android_getCpuFamily();
    if (cpuFamily == ANDROID_CPU_FAMILY_ARM)
	{
        features = android_getCpuFeatures();
		if (features & ANDROID_CPU_ARM_FEATURE_ARMv7){
		    cpu |= WELS_CPU_ARMv7;
		}
		if (features & ANDROID_CPU_ARM_FEATURE_VFPv3){
		    cpu |= WELS_CPU_VFPv3;
		}
		if (features & ANDROID_CPU_ARM_FEATURE_NEON){
		    cpu |= WELS_CPU_NEON;
		}

	}
    return cpu;

}

#elif defined(APPLE_IOS)

uint32_t wels_cpu_feature_detect() //Need to be updated for the new device of APPLE 
{
	uint32_t       cpu = 0;
	struct utsname systemInfo;
	
	uname (&systemInfo);
	
	if ((0 != strcmp(systemInfo.machine, "iPhone1,1")) && //iPhone 2G
		(0 != strcmp(systemInfo.machine, "iPhone1,2")) && //iPhone 3G
		(0 != strcmp(systemInfo.machine, "iPod1,1")) &&   //iPod 1G
		(0 != strcmp(systemInfo.machine, "iPod2,1")))     //iPod 2G
	{
		cpu |= WELS_CPU_ARMv7;
		cpu |= WELS_CPU_VFPv3;
		cpu |= WELS_CPU_NEON;
	}
	
	return cpu;
}

#else

uint32_t wels_cpu_feature_detect( uint32_t *number_of_logic_processors )
{
	return 0;
}

#endif


}//namespace WelsHEVCEncoder

