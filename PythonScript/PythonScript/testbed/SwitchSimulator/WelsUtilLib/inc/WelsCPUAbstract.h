/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsCPUAbstract.h
 *
 * \author	James Zhu(jamezhu@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	interfaces for Wels Test Framework
 *
 * \date	4/24/2012  Created by James Zhu
 *
 *************************************************************************************
*/
#ifndef _WELS_CPU_ABSTRACT_H_
#define _WELS_CPU_ABSTRACT_H_

#ifdef    WORDS_BIGENDIAN

static inline uint32_t endian_fix(uint32_t x)
{
    return x;
}

#else 


#ifdef    _MSC_VER
static inline uint32_t endian_fix(uint32_t x)
{
    __asm
    {
        mov   eax,  x
            bswap   eax
            mov   x,    eax
    }
    return x;
}
#else  // GCC
static inline uint32_t endian_fix(uint32_t x)
{
#ifdef ARM_ARCH //Modified by zhaozheng 11-12-2010
    __asm__ __volatile__("rev %0, %0":"+r"(x));
#elif defined (X86_ARCH)
    __asm__ __volatile__("bswap %0":"+r"(x));
#else
    x = ((x & 0xff000000)>> 24) | ((x & 0xff0000) >> 8) |
        ((x & 0xff00) << 8) | ((x&0xff) << 24);
#endif
    return x;
}
#endif  //MSC_VER

#endif  //WORDS_BIGENDIAN

#endif //_WELS_CPU_ABSTRACT_H_


