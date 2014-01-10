/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsTaskThread.cpp
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	define for thread of a task
 *
 * \date	5/09/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/


#include "WelsCommonType.h"
#include "WelsTaskThread.h"

namespace WelsUtilLib {

CWelsTaskThread::CWelsTaskThread(IWelsTaskThreadSink * pSink) : m_pSink(pSink)
{
    m_uiID = (uint32_t)(this);
    m_pTask = NULL;
}


CWelsTaskThread::~CWelsTaskThread()
{
}

void CWelsTaskThread::ExecuteTask()
{
    CWelsAutoLock cLock(m_cLockTask);
    if( m_pSink ){
        m_pSink->OnTaskStart(this, m_pTask);
    }

    if( m_pTask ){
        m_pTask->Execute();
    }

    if( m_pSink ){
        m_pSink->OnTaskStop(this, m_pTask);
    }

    m_pTask = NULL;
}

WelsErrorType CWelsTaskThread::SetTask(WelsUtilLib::IWelsTask *pTask)
{
    CWelsAutoLock cLock(m_cLockTask);

    if( !GetRunning() ){
        return WELS_E_FAIL;
    }

    m_pTask = pTask;

    SignalThread();

    return WELS_S_OK;
}


}

