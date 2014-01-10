/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsTrace.cpp
 *
 * \author	Volvet Zhang(qizh@cisco.com)
 * \modify
 *			
 *			
 *
 * \brief	trace tool
 *
 * \date	4/17/2012  Created by Volvet Zhang
 *
 *************************************************************************************
*/

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "WelsCommonType.h"
#include "WelsTrace.h"
#include "WelsOSAbstract.h"


#ifdef  WIN32
#define  WBX_TRACE_MUDULE_NAME  "wbxtrace.dll"
#elif defined (LINUX) || defined(UNIX)
#define  WBX_TRACE_MUDULE_NAME  "libwbxtrace.so"
#elif defined (MACOS) || (IOS) 
#define  WBX_TRACE_MUDULE_NAME  "wbxtrace.bundle"
#endif

#ifdef  WIN32
#define  WBX_TRACE_DEBUG        "WBXDEBUGA"
#define  WBX_TRACE_INFO         "WBXINFOA"
#define  WBX_TRACE_WARN         "WBXWARNA"
#define  WBX_TRACE_ERROR        "WBXERRORA"

typedef int ( *WBX_TRACE_FUNC )(const char * pStrFormat,  ...);

#else
#define  WBX_TRACE_DEBUG        "WBXDEBUG2"
#define  WBX_TRACE_INFO         "WBXINFO2"
#define  WBX_TRACE_WARN         "WBXWARN2"
#define  WBX_TRACE_ERROR        "WBXERROR2"

typedef int ( *WBX_TRACE_FUNC )(const char * pStrDllName, const char * pStrFormat,  ...);
#endif

namespace WelsUtilLib {


IWelsTrace * IWelsTrace::g_cWelsTracer = NULL;

class  CWelsTraceBase : public IWelsTrace
{
public:
    CWelsTraceBase() 
	{
        m_iMask = WELS_TRACE_LEVEL_ALL;
	}
	virtual ~CWelsTraceBase()
	{
	}

    virtual void SetTraceLevel(int32_t iMask)
    {
        m_iMask = iMask;
    }

	virtual void Trace(int32_t level, const std::string & str)
	{
		std::string  cOutTrace; 

        if( !(level & m_iMask) ){
            return;
        }

		switch(level)
		{
		case WELS_TRACE_LEVEL_INFO:
			cOutTrace = std::string("WELS_INFO: ") + str;
			break;
		case WELS_TRACE_LEVEL_DEBUG:
			cOutTrace = std::string("WELS_DEBUG: ") + str;
			break;
		case WELS_TRACE_LEVEL_WARNING:
			cOutTrace = std::string("WELS_WARNING: ") + str;
			break;
		case WELS_TRACE_LEVEL_ERROR:
		default:
			cOutTrace = std::string("WELS_ERROR: ") + str;
			break;			
		};

		WriteString(level, cOutTrace);
	}

	virtual void WriteString(int32_t level, const std::string & str) = 0;

private:
    int32_t   m_iMask;	
};


class CWelsTraceConsole : public CWelsTraceBase
{
public:
    CWelsTraceConsole()
	{
	}

	virtual ~CWelsTraceConsole()
	{
	}

	virtual void WriteString(int32_t level, const std::string  & str)
	{
		std::cout<<str<<std::endl;
	}
};


class CWelsTraceFile : public CWelsTraceBase
{
public:
	CWelsTraceFile(char * pFileName) 
	{
		m_cFileStream.open(pFileName, std::ios::out);		
	};

	virtual ~CWelsTraceFile()
	{
	};

	virtual void WriteString(int32_t level, const std::string & str)
	{
		m_cFileStream << str << std::endl;
	};

private:
	std::fstream  m_cFileStream;
};



class CWelsTraceWbx : public CWelsTraceBase
{
public:
    CWelsTraceWbx() : 
        m_pfInfoTrace(NULL), 
        m_pfDebugTrace(NULL),
        m_pfWarningTrace(NULL),
        m_pfErrorTrace(NULL)
    {
        m_cModuleHandle = CWelsOSAbstract::WelsLoadLibrary(WBX_TRACE_MUDULE_NAME);

        if( m_cModuleHandle ){
            m_pfInfoTrace = (WBX_TRACE_FUNC)CWelsOSAbstract::WelsGetProcAddress(m_cModuleHandle,
                WBX_TRACE_INFO);
            m_pfDebugTrace =  (WBX_TRACE_FUNC)CWelsOSAbstract::WelsGetProcAddress(m_cModuleHandle,
                WBX_TRACE_DEBUG);
            m_pfWarningTrace =  (WBX_TRACE_FUNC)CWelsOSAbstract::WelsGetProcAddress(m_cModuleHandle,
                WBX_TRACE_WARN);
            m_pfErrorTrace = (WBX_TRACE_FUNC)CWelsOSAbstract::WelsGetProcAddress(m_cModuleHandle,
                WBX_TRACE_ERROR);            
        }
    }

    virtual ~CWelsTraceWbx()
    {
        if( m_cModuleHandle != NULL ){
            CWelsOSAbstract::WelsFreeLibrary(m_cModuleHandle);
        }
    }

    virtual void WriteString(int32_t level, const std::string & str)
    {     
        WBX_TRACE_FUNC func;

        switch(level)
        {
        case WELS_TRACE_LEVEL_INFO:   
            func = m_pfInfoTrace;
            break;
        case WELS_TRACE_LEVEL_DEBUG:
            func = m_pfDebugTrace;
            break;
        case WELS_TRACE_LEVEL_WARNING:
            func = m_pfWarningTrace;
            break;
        case WELS_TRACE_LEVEL_ERROR:
        default:
            func = m_pfErrorTrace;
            break;
        }

        if( func ){
            func(
#ifndef WIN32
                "HEVC CODEC",
#endif
                str.c_str() );
        }
    } 

private:
    WELS_MODULE_HANDLE  m_cModuleHandle;
    WBX_TRACE_FUNC      m_pfInfoTrace;
    WBX_TRACE_FUNC      m_pfDebugTrace;
    WBX_TRACE_FUNC      m_pfWarningTrace;
    WBX_TRACE_FUNC      m_pfErrorTrace;
};


void IWelsTrace::SetLevel(int32_t iMask)
{
    if( g_cWelsTracer != NULL ){
        g_cWelsTracer->SetTraceLevel(iMask);
    }
}

void IWelsTrace::WelsTrace(int32_t level, std::stringstream & stream)
{
	if( g_cWelsTracer != NULL ){
		g_cWelsTracer->Trace(level, stream.str());
	}
}

void IWelsTrace::Init(int32_t type,  void * pParam)
{
	if( g_cWelsTracer != NULL ){
		delete g_cWelsTracer;
		g_cWelsTracer = NULL;
	}

	switch(type)
	{
	case WELS_TRACE_WBX:
        g_cWelsTracer = new CWelsTraceWbx();
		break;
	case WELS_TRACE_FILE:
		g_cWelsTracer = new CWelsTraceFile(static_cast<char*>(pParam));
		break;
	case WELS_TRACE_CONSOLE:
	default:
		g_cWelsTracer = new CWelsTraceConsole();
		break;
	}	
}

void IWelsTrace::Uninit()
{
	if( g_cWelsTracer != NULL ){
		delete g_cWelsTracer;
		g_cWelsTracer = NULL;
	} 
}


}


