/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsLock.h
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	Auto lock
 *
 * \date	4/23/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#ifndef _WELS_LOCK_H_
#define _WELS_LOCK_H_



#include "WelsCommonType.h"
#include "WelsUtility.h"
#include "WelsOSAbstract.h"


namespace WelsUtilLib {

class CWelsLock
{
    DISALLOW_COPY_AND_ASSIGN(CWelsLock);
public:
    CWelsLock()
    {
        CWelsOSAbstract::WelsMutexInit(&m_cMutex);
    }

    virtual ~CWelsLock()
    {
        CWelsOSAbstract::WelsMutexDestroy(&m_cMutex);
    }

    WelsErrorType  Lock()
    {
        return CWelsOSAbstract::WelsMutexLock(&m_cMutex);
    }

    WelsErrorType Unlock()
    {
        return CWelsOSAbstract::WelsMutexUnlock(&m_cMutex);
    }

private:
    WELS_MUTEX   m_cMutex;
};

class CWelsAutoLock
{
    DISALLOW_COPY_AND_ASSIGN(CWelsAutoLock);
public:
    CWelsAutoLock(CWelsLock & cLock) : m_cLock(cLock)
    {
        m_cLock.Lock();
    }

    virtual ~CWelsAutoLock()
    {
        m_cLock.Unlock();
    }

private:
    CWelsLock   & m_cLock;
};

}

#endif







