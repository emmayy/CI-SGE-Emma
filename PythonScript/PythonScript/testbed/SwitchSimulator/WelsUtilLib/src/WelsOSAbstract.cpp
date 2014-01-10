/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsOSAbstract.cpp
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	interfaces for Wels Test Framework
 *
 * \date	4/20/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#ifdef  LINUX
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#endif

#include <string>

#include "WelsCommonType.h"
#include "WelsOSAbstract.h"


namespace WelsUtilLib {

#ifdef  WIN32


WelsErrorType  CWelsOSAbstract::WelsMutexInit(WELS_MUTEX * pMutex)
{
    InitializeCriticalSection(pMutex);

    return WELS_S_OK;
}

WelsErrorType  CWelsOSAbstract::WelsMutexLock(WELS_MUTEX *pMutex)
{
    EnterCriticalSection(pMutex);

    return WELS_S_OK;
}


WelsErrorType CWelsOSAbstract::WelsMutexUnlock(WELS_MUTEX *pMutex)
{
    LeaveCriticalSection(pMutex);

    return WELS_S_OK;
}

WelsErrorType CWelsOSAbstract::WelsMutexDestroy(WELS_MUTEX *pMutex)
{
    DeleteCriticalSection(pMutex);

    return WELS_S_OK;
}


WelsErrorType CWelsOSAbstract::WelsEventInit(WELS_EVENT * pEvent)
{
    WELS_EVENT   h = CreateEvent(NULL, FALSE, FALSE, NULL);

	if( h == NULL ){
		return WELS_E_FAIL;
	}
	*pEvent = h;
	return WELS_S_OK;
}

WelsErrorType CWelsOSAbstract::WelsEventSignal(WELS_EVENT *pEvent)
{
    if( SetEvent( *pEvent ) ){
		return WELS_S_OK;
	}
	return WELS_E_FAIL;
}

WelsErrorType CWelsOSAbstract::WelsEventWait(WELS_EVENT *pEvent)
{
    WaitForSingleObject(*pEvent, INFINITE );

	return WELS_S_OK;
}

WelsErrorType CWelsOSAbstract::WelsEventDestroy(WELS_EVENT *pEvent)
{
    CloseHandle( *pEvent );

	*pEvent = NULL;
	return WELS_S_OK;
}

WelsErrorType  CWelsOSAbstract::WelsThreadCreate(WELS_THREAD_HANDLE *pThread, LPWELS_THREAD_ROUTINE routine, void *arg, WELS_THREAD_ATTR attr)
{
    WELS_THREAD_HANDLE h = (WELS_THREAD_HANDLE)(_beginthreadex(NULL, 0, routine, arg, 0, NULL));

    if( h <= 0 ){
        return WELS_E_FAIL;
    }

    *pThread = h;

    return WELS_S_OK;
}

WelsErrorType CWelsOSAbstract::WelsThreadJoin(WELS_THREAD_HANDLE thread)
{
    WaitForSingleObject(thread, INFINITE);

    return WELS_S_OK;
}

WelsErrorType CWelsOSAbstract::WelsThreadDestroy(WELS_THREAD_HANDLE thread)
{
    return WELS_S_OK;
}

void CWelsOSAbstract::WelsSleep(uint32_t dwMilliSecond)
{
    ::Sleep(dwMilliSecond);
}


WELS_MODULE_HANDLE  CWelsOSAbstract::WelsLoadLibrary(const char * pModuleName)
{
    return ::LoadLibraryA(pModuleName);
}

void * CWelsOSAbstract::WelsGetProcAddress(WELS_MODULE_HANDLE hModule, const char *pProcName)
{
    return ::GetProcAddress(static_cast<HMODULE>(hModule), pProcName);
}

void  CWelsOSAbstract::WelsFreeLibrary(WELS_MODULE_HANDLE hModule)
{
    ::FreeLibrary(static_cast<HMODULE>(hModule));
}

uint32_t CWelsOSAbstract::WelsNow()
{
    return timeGetTime();
}

uint32_t CWelsOSAbstract::WelsAccuracyGetTime()
{
    static uint64_t  uiFrequency = 0;
	uint64_t         uiTime;

	if( uiFrequency == 0 ){
		QueryPerformanceFrequency((LARGE_INTEGER*)(&uiFrequency));
	}

	QueryPerformanceCounter((LARGE_INTEGER*)(&uiTime));

    return static_cast<uint32_t>(uiTime*1000.0/uiFrequency + 0.5);
}

#else 


WelsErrorType CWelsOSAbstract::WelsMutexInit(WELS_MUTEX  * pMutex)
{
    return pthread_mutex_init(pMutex, NULL) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsMutexLock(WELS_MUTEX  * pMutex)
{
    return pthread_mutex_lock(pMutex) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsMutexUnlock(WELS_MUTEX * pMutex)
{
    return pthread_mutex_unlock(pMutex) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsMutexDestroy(WELS_MUTEX * pMutex)
{
    return pthread_mutex_destroy(pMutex) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

    

WelsErrorType CWelsOSAbstract::WelsEventInit(WELS_EVENT * pEvent)
{
    std::string  cStrSemName("WelsSem");
    char    strSuffix[16] = { 0 };
    
    sprintf(strSuffix, "0x%x", (uint32_t)pEvent);
    
    cStrSemName += strSuffix;
    
    *pEvent = sem_open(cStrSemName.c_str(), O_CREAT, 0644, 0);
    
    return ((*pEvent) == SEM_FAILED) ? WELS_S_FALSE : WELS_S_OK;
}


WelsErrorType CWelsOSAbstract::WelsEventSignal(WELS_EVENT * pEvent)
{
    return sem_post(*pEvent) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsEventWait(WELS_EVENT * pEvent)
{
    return sem_wait(*pEvent) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsEventDestroy(WELS_EVENT * pEvent)
{
    return sem_close(*pEvent) == 0 ? WELS_S_OK : WELS_S_FALSE;
}


WelsErrorType CWelsOSAbstract::WelsThreadCreate(WELS_THREAD_HANDLE * pThread,  LPWELS_THREAD_ROUTINE  routine, 
										   void * arg, WELS_THREAD_ATTR attr)
{
    return pthread_create(pThread, NULL, routine, arg) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsThreadJoin(WELS_THREAD_HANDLE thread)
{
    return pthread_join(thread, NULL) == 0 ? WELS_S_OK : WELS_S_FALSE;
}

WelsErrorType CWelsOSAbstract::WelsThreadDestroy(WELS_THREAD_HANDLE thread)
{
    return WELS_S_OK;
}


void CWelsOSAbstract::WelsSleep(uint32_t dwMilliSecond)
{
    usleep(dwMilliSecond * 1000);
}


WELS_MODULE_HANDLE  CWelsOSAbstract::WelsLoadLibrary(const char * pModuleName)
{
    return ::dlopen(pModuleName, RTLD_LAZY);
}

void * CWelsOSAbstract::WelsGetProcAddress(WELS_MODULE_HANDLE hModule, const char *pProcName)
{
    return ::dlsym(hModule,  pProcName);
}

void  CWelsOSAbstract::WelsFreeLibrary(WELS_MODULE_HANDLE hModule)
{
    ::dlclose(hModule);
}


uint32_t CWelsOSAbstract::WelsNow()
{
    struct timeval sTime;

    gettimeofday(&sTime, NULL);
    return (sTime.tv_sec*1000 + sTime.tv_usec/1000);
}

uint32_t CWelsOSAbstract::WelsAccuracyGetTime()
{
    return CWelsOSAbstract::WelsNow();
}


#endif


}





