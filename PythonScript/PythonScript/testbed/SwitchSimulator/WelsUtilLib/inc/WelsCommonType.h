/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsCommonType.h
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	Common type defines 
 *
 * \date	4/17/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#ifndef  _WELS_COMMON_TYPE_H_
#define  _WELS_COMMON_TYPE_H_




#ifdef WIN32

#include <windows.h>
#include <process.h>

typedef   signed char       int8_t;
typedef   unsigned char     uint8_t;
typedef   signed short      int16_t;
typedef   unsigned short    uint16_t;
typedef   signed int        int32_t;
typedef   unsigned int      uint32_t;
typedef   signed __int64    int64_t;
typedef   unsigned __int64  uint64_t;

#define	WELS_THREAD_ROUTINE_TYPE		DWORD  WINAPI
#define WELS_THREAD_ROUTINE_RETURN(rc)	return (DWORD)rc;
typedef uint32_t (WINAPI *LPWELS_THREAD_ROUTINE) (void *pParam);

typedef  CRITICAL_SECTION   WELS_MUTEX;
typedef  HANDLE             WELS_THREAD_HANDLE;
typedef  HANDLE             WELS_EVENT;

typedef  HANDLE             WELS_MODULE_HANDLE;

#else

#include <stdint.h>

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>

typedef  pthread_mutex_t    WELS_MUTEX;
typedef  pthread_t          WELS_THREAD_HANDLE;


typedef  sem_t *            WELS_EVENT;


typedef  void *  (*LPWELS_THREAD_ROUTINE)(void * pParam);
#define	 WELS_THREAD_ROUTINE_TYPE		void *
#define  WELS_THREAD_ROUTINE_RETURN(rc)	return (void*)rc;

typedef  void *             WELS_MODULE_HANDLE;

#endif

typedef    int32_t          WELS_THREAD_ATTR;
typedef    bool             bool_t;

//******Declare Input|Output Argument in interface function declaration***********
#define _WELS_IN_
#define _WELS_OUT_
#define _WELS_INOUT_


//******Error Types***********
typedef enum{
  WELS_S_OK = (0x00000000),          // Success codes
  WELS_S_FALSE = (0x00000001),
  WELS_E_FAIL = (0x80000001),        //  Unspecified error
  WELS_E_OUTOFMEMORY = (0x80000002), //  Ran out of memory
  WELS_E_INVALIDARG = (0x80000003),  //  One or more arguments are invalid
  WELS_E_NOTIMPL = (0x80000004),     //  Not implemented
  WELS_E_NOINTERFACE = (0x80000005), //  No such interface supported
  WELS_E_POINTER = (0x80000006),     //  Invalid pointer  
}WelsErrorType;


#define  WELS_SUCCEED(rt) (rt == WELS_S_OK || rt == WELS_S_FALSE)

#define  WELS_FAILED(rt)    (!WELS_SUCCEED(rt))

#define  WELS_ASSERT_RETURN(expression) \
    rt = expression;\
    if( WELS_FAILED(rt)){\
        return rt;\
    }

#endif






