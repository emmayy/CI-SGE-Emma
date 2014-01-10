/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsUtlity.h
 *
 * \author	Volvet Zhang (qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	interfaces for Wels Ultility lib
 *
 * \date	4/17/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#ifndef _WELS_UTILITY_H_
#define _WELS_UTILITY_H_

#include <string>
#include <fstream>

#include "WelsCommonType.h"

#define MAX_INT       (32767)//TBD
#define MIN_INT       (-32768)//TBD

#define MAX_INT16       (32767)
#define MIN_INT16       (-32768)

//#define WELS_MIN(a,b) ((a) < (b)) ? (a) : (b)
#define WELS_MIN(x,y) ((y) ^ (((x)^(y))& -((x)<(y))))
#define WELS_MIN_FLOAT(a,b) ((a) < (b)) ? (a) : (b)

//#define WELS_MAX(a,b) ((a) > (b)) ? (a) : (b)
#define WELS_MAX(x,y) ((x) ^ (((x)^(y))& -((x)<(y))))
#define WELS_MAX_FLOAT(a,b) ((a) > (b)) ? (a) : (b)

//#define WELS_CLIP3(a,b,x) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define WELS_CLIP3(a,b,x) ( WELS_MAX( (a), WELS_MIN( (b), (x) ) ) )
#define WELS_CLIP3_FLOAT(a,b,x) (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))

//#define WELS_CLIP(x) WELS_CLIP3(0,255,(x))
#define WELS_CLIP(x) (((x) & ~255) ? (-(x) >> 31) : (x))

#define WELS_SIGN(a)  ((int32_t) (a) >> 31)

#define WELS_ROUND(a)  ( static_cast<int>(a+0.5f) )

#define WELS_ALIGN(x, n)	(((x)+(n)-1)&~((n)-1))

#define WELS_UINT_FILTER121(a,b,c)  ( (a + c + (b<<1) + 2)>>2  )

//  For 32/16/8 bits int
#define WELS_ABS(a)   ((WELS_SIGN(a) ^ (int32_t)(a)) - WELS_SIGN(a))

//  for 32/16/8 bit int
#define  WELS_RESTORE_SIGN(a, sign) ((int32_t)(a + sign) ^ sign)
#define  WELS_ABS_WITH_SIGN(a, sign) ((sign ^ (int32_t)a) - sign)

#define DISALLOW_COPY_AND_ASSIGN(cclass) \
private:	\
    cclass(const cclass &);	\
    cclass& operator=(const cclass &);


#ifndef WelsSafeDelete
#define WelsSafeDelete(p)\
if(p)\
{\
    delete p;\
    p = NULL;\
}
#endif

#ifndef WelsSafeDeleteArray
#define WelsSafeDeleteArray(p)\
if(p)\
{\
    delete[] p;\
    p = NULL;\
}
#endif

#ifndef WelsSafeRelease
#define WelsSafeRelease(p)\
if(p)\
{\
    p->Release();\
    p = NULL;\
}
#endif

#define WelsCheckedFalseReturnVoid(p)\
if(!p){\
    return;\
}

#define WelsCheckedFalseReturn(p,rt)\
if (!p){\
    return rt;\
}

#define WelsFailedReturn(p)\
rt = (p);\
if(rt != WELS_S_OK && rt != WELS_S_FALSE) \
{ \
	return rt;\
}


namespace WelsUtilLib{

#if defined(_WIN64) || defined(__LP64__)
inline int64_t TranslatePtrToInt(void* ptr)
{
    return (int64_t)(ptr);
}
#else
inline int32_t TranslatePtrToInt(void* ptr)
{
    return (int32_t)(ptr);
}
#endif


inline uint32_t Wels_log2_Plus1(const uint32_t iValue)
{
    uint32_t iLen = 1;
    uint32_t iV = iValue;
    while (iV>1){
        iV >>= 1;
        ++iLen;
    }

    return iLen;
}

template<typename T> 
inline void WelsSwap(T& _Left, T& _Right)
{	// exchange values stored at _Left and _Right
        T _Tmp = _Left;
        _Left   = _Right;
        _Right  = _Tmp;
}

}//namespace WelsUtilLib

#endif//_WELS_UTILITY_H_

