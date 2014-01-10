/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsThread.cpp
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	define for thread
 *
 * \date	5/09/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#include "WelsCommonType.h"
#include "WelsThread.h"
#include "WelsTrace.h"

namespace WelsUtilLib {

CWelsThread::CWelsThread() : 
    m_bRunning(false), 
    m_bEndFlag(false),
    m_hThread(0)     
{    
    WelsErrorType rc = CWelsOSAbstract::WelsEventInit(&m_hEvent);

    if( WELS_S_OK != rc ){
        WELS_ERROR_TRACE("Create Event failed!!!");
    }
}

CWelsThread::~CWelsThread()
{
    Kill();

    //if( m_hEvent){
    CWelsOSAbstract::WelsEventDestroy(&m_hEvent);
    //}
}


void CWelsThread::Thread()
{
    while(true) {
        CWelsOSAbstract::WelsEventWait(&m_hEvent);

        if( GetEndFlag() ){
            break;
        }

        ExecuteTask();
    }

    SetRunning(false);
}

WelsErrorType CWelsThread::Start()
{
    if( GetRunning() ){
        return WELS_S_OK;
    }

    SetEndFlag(false);

    WelsErrorType rc = CWelsOSAbstract::WelsThreadCreate(&m_hThread, 
        (LPWELS_THREAD_ROUTINE)TheThread, this, 0);

    if( WELS_S_OK != rc ){
        return rc;
    }

    while( !GetRunning() ){
        CWelsOSAbstract::WelsSleep(1);
    }

    return WELS_S_OK;
}

void CWelsThread::Kill()
{
    if( !GetRunning() ){
        return;
    }

    SetEndFlag(true);

    CWelsOSAbstract::WelsEventSignal(&m_hEvent);

    CWelsOSAbstract::WelsThreadJoin(m_hThread);

    return;    
}

WELS_THREAD_ROUTINE_TYPE  CWelsThread::TheThread(void * pParam)
{
    CWelsThread * pThis = static_cast<CWelsThread*>(pParam);

    pThis->SetRunning(true);

    pThis->Thread();

    WELS_THREAD_ROUTINE_RETURN(NULL);
}

}


