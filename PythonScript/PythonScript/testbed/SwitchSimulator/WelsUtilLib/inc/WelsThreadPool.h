/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsThreadPool.h
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	Thread Pool
 *
 * \date	5/09/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/


#ifndef _WELS_THREAD_POOL_H_
#define _WELS_THREAD_POOL_H_

#include <map>

#include "WelsTask.h"
#include "WelsTaskThread.h"


namespace WelsUtilLib {


class IWelsThreadPoolSink 
{
public:
    virtual WelsErrorType  OnTaskExecuted(IWelsTask * pTask) = 0;
    virtual WelsErrorType  OnTaskCancelled(IWelsTask * pTask) = 0;
};


class  CWelsThreadPool : public CWelsThread,  public IWelsTaskThreadSink
{
public:
    enum {
        DEFAULT_THREAD_NUM = 4,
    };

    CWelsThreadPool(IWelsThreadPoolSink * pSink = NULL, int32_t iMaxThreadNum = DEFAULT_THREAD_NUM);
    virtual ~CWelsThreadPool();

    //IWelsTaskThreadSink
    virtual WelsErrorType OnTaskStart(CWelsTaskThread * pThread,  IWelsTask * pTask);
    virtual WelsErrorType OnTaskStop(CWelsTaskThread * pThread,  IWelsTask * pTask);

    //  CWelsThread 
    virtual void ExecuteTask();
    
    WelsErrorType  QueueTask(IWelsTask * pTask);
    int32_t        GetThreadNum() const 
    {
        return m_iMaxThreadNum;
    }


protected:
    WelsErrorType  Init(int32_t iMaxThreadNum = DEFAULT_THREAD_NUM);
    WelsErrorType  Uninit();

    WelsErrorType  CreateIdleThread();
    void           DestroyThread(CWelsTaskThread * pThread);
    void           AddThreadToIdleMap(CWelsTaskThread * pThread);
    void           AddThreadToBusyMap(CWelsTaskThread * pThread);
    void           RemoveThreadFromBusyMap(CWelsTaskThread * pThread);
    void           AddTaskToWaitedList(IWelsTask * pTask);
    CWelsTaskThread  * GetIdleThread();
    IWelsTask        * GetWaitedTask();
    int32_t            GetIdleThreadNum(); 
    int32_t            GetBusyThreadNum();
    int32_t            GetWaitedTaskNum();
    void               ClearWaitedTasks();



private:
    int32_t   m_iMaxThreadNum;
    std::list<IWelsTask *>    m_cWaitedTasks; 
    std::map<uint32_t, CWelsTaskThread*>  m_cIdleThreads;
    std::map<uint32_t, CWelsTaskThread*>  m_cBusyThreads;
    IWelsThreadPoolSink  * m_pSink;

    CWelsLock   m_cLockPool;
    CWelsLock   m_cLockWaitedTasks;
    CWelsLock   m_cLockIdleTasks;
    CWelsLock   m_cLockBusyTasks;

    DISALLOW_COPY_AND_ASSIGN(CWelsThreadPool);
};

}


#endif



