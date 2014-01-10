/** 
*************************************************************************************
* \copy	Copyright (C) 2013 by cisco, Inc.
*
*
* \file	WelsMemory.cpp
*
* \author	Derrick Jin (xingji@cisco.com)
* \modify
*			
*			
*
* \memory tool, to alloc memory align to 16 bytes, and record all alloc info to avoid memory leak
*
* \date	4/18/2012  Created by Derrick Jin
*
*************************************************************************************
*/
#include <string.h>

#include "WelsCommonType.h"
#include "WelsTrace.h"
#include "WelsMemory.h"
#include "WelsUtility.h"

namespace WelsUtilLib{

CWelsMemory::CWelsMemory(int32_t iCache_line_size ,bool_t bEnableMemCheck )
{
	m_memMap.clear();
	m_iCache_line_size = iCache_line_size;
	m_bEnableMemCheck = bEnableMemCheck;
}

CWelsMemory::~CWelsMemory()
{
    UnInitialize();
}

int32_t CWelsMemory::UnInitialize()
{
    if (!m_memMap.empty()){
        for (MemMapIter iter = m_memMap.begin(); iter!= m_memMap.end();++iter){
            WELS_ERROR_TRACE("undelete resource "<<(*iter).first<<" "<<(*iter).second);
        }
        m_memMap.clear();
        return WELS_E_FAIL;
    }
    
	if( !m_cppSytleMemMap.empty() ){
		for( MemMapIter iter = m_cppSytleMemMap.begin(); iter != m_cppSytleMemMap.end(); ++ iter ){
			WELS_ERROR_TRACE("undelete resource "<<(*iter).first<<" "<<(*iter).second);
		}
	}

    return WELS_S_OK;
}
bool_t CWelsMemory::Insert(void* p,const int32_t iLen,const std::string& strFile,const int32_t iLine, const std::string& strAddInfo)
{
    WelsCheckedFalseReturn(m_bEnableMemCheck,true);

	std::ostringstream ost;
    ost<<"file: "<<strFile<<"  line: "<<iLine<<" ; buffer length "<<iLen<< "; idx:" << m_memMap.size() << " ; add info : "<<strAddInfo;
	
	m_memMap[p] = ost.str();
	return true;
}

bool_t CWelsMemory::Erase(void* _WELS_IN_ p)
{
    WelsCheckedFalseReturn(m_bEnableMemCheck,true);

	int iResult =  m_memMap.erase(p);
	return (iResult)?(true):(false);
}


void * CWelsMemory::InsertCppStyleMem(void* p, const std::string& strFile,const int32_t iLine, const std::string& strAddInfo)
{
    WelsCheckedFalseReturn(m_bEnableMemCheck, p);

	std::ostringstream ost;
    ost<<"file: "<<strFile<<"  line: "<<iLine<< "; idx:" << m_cppSytleMemMap.size() << " ; add info : "<<strAddInfo;
	
	m_cppSytleMemMap[p] = ost.str();        
     
	return p;
}

bool_t CWelsMemory::EraseCppStyleMem(void* _WELS_IN_ p)
{
    WelsCheckedFalseReturn(m_bEnableMemCheck,true);

	int iResult =  m_cppSytleMemMap.erase(p);
	return (iResult)?(true):(false);
}

CWelsMemory* CWelsMemoryInstance::g_pCWelsMemory = NULL;

static int32_t CalcAllignBytes(const int32_t iLen,const int32_t iAlignBytes)
{
	return iLen + iAlignBytes + sizeof(void*);		 
}
void* WelsMallocBase(const int32_t iLen,const std::string& strFile,const int32_t iLine,const std::string& strAddInfo,const bool bZeroMem)
{
	int32_t iAlignBytes = CWelsMemoryInstance::GetCWelsMemoryInstance()->GetCacheLineSize() - 1;
	int32_t iActualLen = CalcAllignBytes(iLen,iAlignBytes);
	int8_t* p = NULL;
	int8_t* pAlign_buf = NULL;
	p = new int8_t[iActualLen];

    if (p){       
	    pAlign_buf = p + iAlignBytes + sizeof(void*);

	    pAlign_buf -= (TranslatePtrToInt(pAlign_buf) & iAlignBytes);
	    *(void**)(pAlign_buf - sizeof(void*)) = p;
	
        CWelsMemoryInstance::GetCWelsMemoryInstance()->Insert(pAlign_buf,iLen,strFile,iLine,strAddInfo);

        if (bZeroMem){
            memset(pAlign_buf,0,iLen);
        }
    }
    return pAlign_buf;
}

int32_t WelsSafeFreeBase(void* p,const std::string& strFile,const int32_t iLine)
{
    WelsCheckedFalseReturn(p,WELS_E_INVALIDARG);

    bool_t bEraseSucceed = CWelsMemoryInstance::GetCWelsMemoryInstance()->Erase(p);
	if (bEraseSucceed){
        int8_t* pTemp = static_cast<int8_t*>(p);
        int8_t* pBuf = (int8_t*)( *((void**)(pTemp - sizeof(void*))) );
		WelsSafeDeleteArray(pBuf);
		return WELS_S_OK;
	}else{
		WELS_ERROR_TRACE(" did not found resource "<<p<<" ; file : "<<strFile<<" ; line : "<<iLine);
	}

	return WELS_E_FAIL;
}

}//namespace WelsUtilLib

