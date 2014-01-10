/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsTaskThread.h
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


#ifndef _WELS_TASK_THREAD_H_
#define _WELS_TASK_THREAD_H_


#include "WelsTask.h"
#include "WelsThread.h"

namespace WelsUtilLib {

class CWelsTaskThread;

class IWelsTaskThreadSink
{
public:
    virtual WelsErrorType OnTaskStart(CWelsTaskThread * pThread, IWelsTask * pTask) = 0;
    virtual WelsErrorType OnTaskStop(CWelsTaskThread * pThread, IWelsTask * pTask) = 0;
};

class CWelsTaskThread : public CWelsThread
{
public:
    CWelsTaskThread(IWelsTaskThreadSink * pSink);
    virtual ~CWelsTaskThread();

    WelsErrorType  SetTask(IWelsTask * pTask);
    virtual void ExecuteTask();

    uint32_t    GetID() const 
    {
        return m_uiID;
    }

private:
    CWelsLock   m_cLockTask;
    IWelsTaskThreadSink  * m_pSink;
    IWelsTask   * m_pTask;
    uint32_t    m_uiID;

    DISALLOW_COPY_AND_ASSIGN(CWelsTaskThread);
};

}

#endif

