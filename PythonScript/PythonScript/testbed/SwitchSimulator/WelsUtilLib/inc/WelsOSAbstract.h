/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsOSAbstract.h
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	interfaces for Wels Operation System Abstract Layer
 *
 * \date	4/20/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/


#ifndef  _WELS_OS_ABSTRACT_H_
#define  _WELS_OS_ABSTRACT_H_


#include "WelsCommonType.h"

#define ALWAYS_INLINE inline



namespace WelsUtilLib {

// ***************************************************************************
// Aligned Alloc Variables Address
// ***************************************************************************
#ifdef  WIN32
#define WELS_DECLARE_ALIGNED( var, n ) __declspec(align(n)) var
#else
#define WELS_DECLARE_ALIGNED( var, n ) var __attribute__((aligned(n)))
#endif
#define WELS_ALIGNED_16( var ) WELS_DECLARE_ALIGNED( var, 16 )
#define WELS_ALIGNED_8( var )  WELS_DECLARE_ALIGNED( var, 8 )
#define WELS_ALIGNED_4( var )  WELS_DECLARE_ALIGNED( var, 4 )

// ***************************************************************************
// OS depended functions
// ***************************************************************************
class CWelsOSAbstract
{
public:

    //  Thread interfaces
static WelsErrorType    WelsMutexInit( WELS_MUTEX   * pMutex );
static WelsErrorType    WelsMutexLock( WELS_MUTEX   * pMutex );
static WelsErrorType    WelsMutexUnlock( WELS_MUTEX * pMutex );
static WelsErrorType    WelsMutexDestroy( WELS_MUTEX * pMutex );

static WelsErrorType    WelsEventInit( WELS_EVENT  * pEvent );
static WelsErrorType    WelsEventSignal( WELS_EVENT * pEvent );
static WelsErrorType    WelsEventWait( WELS_EVENT * pEvent );
static WelsErrorType    WelsEventDestroy( WELS_EVENT * pEvent );

static WelsErrorType    WelsThreadCreate( WELS_THREAD_HANDLE * pThread,  LPWELS_THREAD_ROUTINE  routine, 
										   void * arg, WELS_THREAD_ATTR attr);

static WelsErrorType    WelsThreadJoin( WELS_THREAD_HANDLE  thread );

static WelsErrorType    WelsThreadDestroy( WELS_THREAD_HANDLE thread );

static void             WelsSleep(uint32_t dwMilliSecond);

    // Module interface
static WELS_MODULE_HANDLE   WelsLoadLibrary(const char * pModuleName);
static void *               WelsGetProcAddress(WELS_MODULE_HANDLE  hModule,  const char * pProcName);
static void                 WelsFreeLibrary(WELS_MODULE_HANDLE hModule);

    //time
static  uint32_t        WelsNow();
static  uint32_t        WelsAccuracyGetTime();
};



}




#endif



