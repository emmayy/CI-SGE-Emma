/** 
 *************************************************************************************
 * \copy	Copyright (C) 2013 by cisco, Inc.
 *
 *
 * \file	WelsTrace.h
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


#ifndef _WELS_TRACE_H_
#define _WELS_TRACE_H_

#include <sstream>

#include "WelsCommonType.h"

namespace WelsUtilLib {



class IWelsTrace {
public:
	enum {
		WELS_TRACE_LEVEL_INFO = 0x1, 
		WELS_TRACE_LEVEL_DEBUG = 0x2, 
		WELS_TRACE_LEVEL_WARNING = 0x4, 
		WELS_TRACE_LEVEL_ERROR = 0x8, 
        WELS_TRACE_LEVEL_ALL = WELS_TRACE_LEVEL_INFO |
                                WELS_TRACE_LEVEL_DEBUG |
                                WELS_TRACE_LEVEL_WARNING |
                                WELS_TRACE_LEVEL_ERROR
    };

	enum ETraceType{
		WELS_TRACE_CONSOLE  = 0, 
		WELS_TRACE_FILE     = 1, 
		WELS_TRACE_WBX      = 2
	};

	virtual ~IWelsTrace() { }

	virtual  void  Trace(int32_t level, const std::string & str) = 0;
    virtual  void  SetTraceLevel(int32_t iMask) = 0;

    static void SetLevel(int32_t iMask);
	static void WelsTrace(int32_t level, std::stringstream & stream);	
	static void Init(int32_t type,  void * pParam);
	static void Uninit();

	static IWelsTrace  * g_cWelsTracer;   
};


#define  WELS_WRITE_TRACE(level, str)   \
	do { \
	    std::stringstream   stream;   \
	    stream << str;  \
		WelsUtilLib::IWelsTrace::WelsTrace(level, stream); \
	} while(0);
	
	

#define  WELS_INFO_TRACE(str)               \
	WELS_WRITE_TRACE(WelsUtilLib::IWelsTrace::WELS_TRACE_LEVEL_INFO,  str) 
#define  WELS_DEBUG_TRACE(str)              \
	WELS_WRITE_TRACE(WelsUtilLib::IWelsTrace::WELS_TRACE_LEVEL_DEBUG,  str)
#define  WELS_WARNING_TRACE(str)            \
	WELS_WRITE_TRACE(WelsUtilLib::IWelsTrace::WELS_TRACE_LEVEL_WARNING,  str)
#define  WELS_ERROR_TRACE(str)              \
	WELS_WRITE_TRACE(WelsUtilLib::IWelsTrace::WELS_TRACE_LEVEL_ERROR,  str)


#define WELS_ERROR_TRACE_POS(str) \
    WELS_ERROR_TRACE("file : "__FILE__<<" line : "<<__LINE__);\
    WELS_ERROR_TRACE(str);

};


#endif


