/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsThreadPool.cpp
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


#include <list>
#include <map>

#include "WelsCommonType.h"
#include "WelsThreadPool.h"
#include "WelsTrace.h"


namespace WelsUtilLib {


CWelsThreadPool::CWelsThreadPool(IWelsThreadPoolSink *pSink, int32_t iMaxThreadNum) : 
    m_pSink(pSink)
{
    m_iMaxThreadNum = 0;

    Init(iMaxThreadNum);
}


CWelsThreadPool::~CWelsThreadPool()
{
    Uninit();
}

WelsErrorType CWelsThreadPool::OnTaskStart(CWelsTaskThread *pThread, IWelsTask *pTask)
{
    AddThreadToBusyMap(pThread);
    
    return WELS_S_OK;
}

WelsErrorType CWelsThreadPool::OnTaskStop(CWelsTaskThread *pThread, IWelsTask *pTask)
{
    RemoveThreadFromBusyMap(pThread);
    AddThreadToIdleMap(pThread);

    if( m_pSink ){
        m_pSink->OnTaskExecuted(pTask);
    }

    SignalThread();
    return WELS_S_OK;
}

WelsErrorType CWelsThreadPool::Init(int32_t iMaxThreadNum)
{
    CWelsAutoLock  cLock(m_cLockPool);
    //WELS_INFO_TRACE("Enter WelsThreadPool Init");

    int32_t i;

    if( iMaxThreadNum <= 0 )  iMaxThreadNum = 1;
    m_iMaxThreadNum = iMaxThreadNum;

    for( i=0;i<m_iMaxThreadNum;i++ ){
        if( WELS_S_OK != CreateIdleThread() ){
            WELS_ERROR_TRACE("CWelsThreadPool::Init - Create Idle thread failed");
        }
    }

    Start();

    //WELS_INFO_TRACE("Leave WelsThreadPool Init");
    return WELS_S_OK;
}

WelsErrorType CWelsThreadPool::Uninit()
{
    CWelsAutoLock  cLock(m_cLockPool);

    ClearWaitedTasks();

    while( GetBusyThreadNum() > 0 ){
        WELS_INFO_TRACE("CWelsThreadPool::Uninit - Waiting all thread to exit");

        CWelsOSAbstract::WelsSleep(10);
    }

    if( GetIdleThreadNum() != m_iMaxThreadNum ){
        WELS_WARNING_TRACE("CWelsThreadPool::Uninit - thread number wrong!!!");
    }

    m_cLockIdleTasks.Lock();
    std::map<uint32_t, CWelsTaskThread*>::iterator iter = m_cIdleThreads.begin();
    while( iter != m_cIdleThreads.end() ){
        DestroyThread(iter->second);
        ++ iter;
    }
    m_cLockIdleTasks.Unlock();
    
    m_iMaxThreadNum = 0;
    Kill();

    return WELS_S_OK;
}

void CWelsThreadPool::ExecuteTask()
{
   	CWelsTaskThread *pThread = NULL;
	IWelsTask    *pTask = NULL;
	while ( GetWaitedTaskNum() > 0 ){
		pThread = GetIdleThread();
		if( pThread == NULL ){
			break;
		}
		pTask = GetWaitedTask();
        //WELS_INFO_TRACE("ThreadPool:  ExecuteTask = "<<(uint32_t)(pTask));
		pThread->SetTask(pTask);		
	}
}

WelsErrorType CWelsThreadPool::QueueTask(IWelsTask *pTask)
{
    CWelsAutoLock  cLock(m_cLockPool);

    //WELS_INFO_TRACE("ThreadPool:  QueueTask = "<<(uint32_t)(pTask));

    if( GetWaitedTaskNum() == 0 ){
        CWelsTaskThread * pThread = GetIdleThread();

        if( pThread != NULL ){
            //WELS_INFO_TRACE("ThreadPool:  ExecuteTask = "<<(uint32_t)(pTask));
            pThread->SetTask(pTask);

            return WELS_S_OK;
        } 
    }

    AddTaskToWaitedList(pTask);   

    return WELS_S_OK;
}

WelsErrorType CWelsThreadPool::CreateIdleThread()
{
    CWelsTaskThread * pThread = new CWelsTaskThread(this);

    if( NULL == pThread ){
        WELS_ERROR_TRACE("CWelsThreadPool::CreateIdleThread - Failed to create thread");
        return WELS_E_FAIL;
    }

    pThread->Start();
    AddThreadToIdleMap(pThread);

    return WELS_S_OK;
}

void  CWelsThreadPool::DestroyThread(CWelsTaskThread * pThread)
{
    pThread->Kill();
    delete pThread;

    return;
}

void  CWelsThreadPool::AddThreadToIdleMap(CWelsTaskThread * pThread)
{
    CWelsAutoLock cLock(m_cLockIdleTasks);

    uint32_t id = pThread->GetID();

    std::map<uint32_t, CWelsTaskThread*>::iterator  iter = m_cIdleThreads.find(id);

    if( iter != m_cIdleThreads.end() ){
        WELS_WARNING_TRACE("CWelsThreadPool::AddThreadToIdleMap - the thread already exist in idle map");
        return;
    }

    m_cIdleThreads[id] = pThread;

    return;
}

void  CWelsThreadPool::AddThreadToBusyMap(CWelsTaskThread * pThread)
{
    CWelsAutoLock cLock(m_cLockBusyTasks);

    uint32_t id = pThread->GetID();

    std::map<uint32_t, CWelsTaskThread*>::iterator iter = m_cBusyThreads.find(id);

    if( iter != m_cBusyThreads.end() ){
        WELS_WARNING_TRACE("CWelsThreadPool::AddThreadToBusyMap - the thread already exist in the busy map");
        return;
    }

    m_cBusyThreads[id] = pThread;

    return;
}

void  CWelsThreadPool::RemoveThreadFromBusyMap(CWelsTaskThread * pThread)
{
    CWelsAutoLock cLock(m_cLockBusyTasks);

    uint32_t  id = pThread->GetID();

    std::map<uint32_t, CWelsTaskThread*>::iterator iter = m_cBusyThreads.find(id);

    if( iter != m_cBusyThreads.end() ){
        m_cBusyThreads.erase(iter);
    } else {
        WELS_WARNING_TRACE("CWelsThreadPool::RemoveThreadFromBusyMap - Can't find the thread");
    }

    return;
}

void  CWelsThreadPool::AddTaskToWaitedList(IWelsTask * pTask)
{
    CWelsAutoLock  cLock(m_cLockWaitedTasks);

    m_cWaitedTasks.push_back(pTask);
    return;
}

CWelsTaskThread *  CWelsThreadPool::GetIdleThread()
{
    CWelsAutoLock cLock(m_cLockIdleTasks);

	if( m_cIdleThreads.size() == 0 ){
		return NULL;
	}

	std::map<uint32_t,CWelsTaskThread*>::iterator it = m_cIdleThreads.begin();
	CWelsTaskThread *pThread = it->second;	

	m_cIdleThreads.erase(it);
	
	return pThread;
}

int32_t  CWelsThreadPool::GetBusyThreadNum()
{
    return m_cBusyThreads.size();
}

int32_t  CWelsThreadPool::GetIdleThreadNum()
{
    return m_cIdleThreads.size();
}

int32_t  CWelsThreadPool::GetWaitedTaskNum()
{
    return m_cWaitedTasks.size();
}

IWelsTask * CWelsThreadPool::GetWaitedTask()
{
    CWelsAutoLock lock(m_cLockWaitedTasks);
	
	if( m_cWaitedTasks.size() == 0 ){
		return NULL;
	}
	
	std::list<IWelsTask*>::iterator it = m_cWaitedTasks.begin();
	IWelsTask *pTask = *it;
	
	m_cWaitedTasks.pop_front();

	return pTask;
}

void  CWelsThreadPool::ClearWaitedTasks()
{
    CWelsAutoLock cLock(m_cLockWaitedTasks);

    std::list<IWelsTask*>::iterator iter = m_cWaitedTasks.begin();

    if( m_pSink ){
        while( iter != m_cWaitedTasks.end() ){
            m_pSink->OnTaskCancelled(*iter);
            ++ iter;
        }
    }

    m_cWaitedTasks.clear();
}

}


