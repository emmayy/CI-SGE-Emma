/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsThread.h
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


#ifndef _WELS_THREAD_H_
#define _WELS_THREAD_H_


#include "WelsOSAbstract.h"
#include "WelsLock.h"

namespace WelsUtilLib {

class CWelsThread 
{
public:
    CWelsThread();
    virtual ~CWelsThread();

    virtual void Thread();
    virtual void ExecuteTask() = 0;
    virtual WelsErrorType Start();
    virtual void Kill();

protected:
    static WELS_THREAD_ROUTINE_TYPE  TheThread(void * pParam);

    void         SetRunning(bool bRunning)
    {
        CWelsAutoLock  cLock(m_cLockStatus);

        m_bRunning = bRunning;
    }
    void         SetEndFlag(bool bEndFlag)
    {
        CWelsAutoLock  cLock(m_cLockStatus);

        m_bEndFlag = bEndFlag;
    }

    bool_t         GetRunning() const 
    {
        return m_bRunning;
    }

    bool_t         GetEndFlag() const 
    {
        return m_bEndFlag;
    }

    void           SignalThread()
    {
        CWelsOSAbstract::WelsEventSignal(&m_hEvent);
    }

private:
    WELS_THREAD_HANDLE  m_hThread;
    WELS_EVENT          m_hEvent;
    CWelsLock           m_cLockStatus;   
    bool_t              m_bRunning;
    bool_t              m_bEndFlag;

    DISALLOW_COPY_AND_ASSIGN(CWelsThread);
};


}



#endif


