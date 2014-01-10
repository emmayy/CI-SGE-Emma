/** 
*************************************************************************************
* \copy	Copyright (C) 2013 by cisco, Inc.
*
*
* \file	WelsMemory.h
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



/************************************************************************************
*using description
*use CWelsMemoryInstance in the class


*WelsMemory instance is a global resource.
*the undeleted memory information will be print out when WelsMemory is not occupied.


*************************************************************************************
*/
#ifndef _WELS_MEMORY_H_
#define _WELS_MEMORY_H_

#include <map>
#include <iostream>
#include <sstream>

#include "WelsCommonType.h"

#include "WelsLock.h"
#include "WelsUnknown.h"

namespace WelsUtilLib{

typedef std::map<void*,std::string> MemMap;
typedef MemMap::iterator       MemMapIter;
typedef std::pair<MemMapIter, bool_t> Insert_Pair;

class CWelsMemory:public CWelsUnknown
{
public:
	CWelsMemory(int32_t iCache_line_size = 16,bool_t bEnableMemCheck = true);
	~CWelsMemory();

    MemMap& GetMap()    { return m_memMap;}
	void SetCacheLineSize(int32_t iCache_line_size)	{	m_iCache_line_size = iCache_line_size;	}
	int32_t GetCacheLineSize()	{	return m_iCache_line_size;	}

    bool_t Insert(void* _WELS_IN_ p,const int32_t iLen,const std::string& sFile,const int32_t nLine, const std::string& addInfo);
	bool_t Erase(void* _WELS_IN_ p);

	void * InsertCppStyleMem(void* _WELS_IN_ p,const std::string& sFile,const int32_t nLine, const std::string& addInfo);
	bool_t EraseCppStyleMem(void * _WELS_IN_ p);

protected:
    int32_t UnInitialize();
private:
    MemMap m_memMap;
	MemMap m_cppSytleMemMap;

	int32_t m_iCache_line_size;
	bool_t m_bEnableMemCheck;
};

class CWelsMemoryInstance
{
public:
    CWelsMemoryInstance()
    {
        GetCWelsMemoryInstance()->AddRef();
    }

    ~CWelsMemoryInstance()
    {      
        int32_t lRef = 0;
        if(g_pCWelsMemory){
            lRef = g_pCWelsMemory->Release();
            if (lRef == 0){
                g_pCWelsMemory = NULL;
            }
        }
    }

    static CWelsMemory* GetCWelsMemoryInstance()
    {
        if (g_pCWelsMemory == NULL){
            g_pCWelsMemory = new CWelsMemory();
        } 

        return g_pCWelsMemory;
    }

    static CWelsMemory* g_pCWelsMemory;
};

void* WelsMallocBase(const int32_t iLen,const std::string& strFile,const int32_t iLine,const std::string& strAddInfo,const bool bZeroMem = false);

int32_t WelsSafeFreeBase(void* p,const std::string& strFile,const int32_t iLine);

}//namespace WelsUtilLib

#ifndef WelsMallocS
#define WelsMallocS(iLen,strAddInfo)  WelsUtilLib::WelsMallocBase(iLen,std::string(__FILE__),__LINE__,strAddInfo)
#endif

#ifndef WelsMalloc
#define WelsMalloc(iLen) WelsMallocS(iLen,std::string(""))
#endif

#ifndef WelsMallocZeroS
#define WelsMallocZeroS(iLen,strAddInfo)  WelsUtilLib::WelsMallocBase(iLen,std::string(__FILE__),__LINE__,strAddInfo,true)
#endif

#ifndef WelsMallocZero
#define WelsMallocZero(iLen) WelsMallocZeroS(iLen,std::string(""))
#endif

#ifndef WelsSafeFree
#define WelsSafeFree(p)\
if(p)\
{\
    WelsUtilLib::WelsSafeFreeBase(p,std::string(__FILE__),__LINE__);\
    p = NULL;\
}
#endif


#define  WELS_NEW_OP(object, type)   \
    (type*)(WelsUtilLib::CWelsMemoryInstance::GetCWelsMemoryInstance()->InsertCppStyleMem(new object, __FILE__, __LINE__, ""));  

#define  WELS_DELETE_OP(p)  \
    WelsUtilLib::CWelsMemoryInstance::GetCWelsMemoryInstance()->EraseCppStyleMem(p);  \
	delete p;            \
	p = NULL;

#define  WELS_DELETE_ARRAY_OP(p)  \
    WelsUtilLib::CWelsMemoryInstance::GetCWelsMemoryInstance()->EraseCppStyleMem(p);  \
    delete [] p;            \
    p = NULL;





#endif //_CWELSMEMORY_H_

