/** 
*************************************************************************************
* \copy	Copyright (C) 2013 by cisco, Inc.
*
*
* \file	WelsUnknown.h
*
* \author	Derrick Jin (xingji@cisco.com)
* \modify
*			
*			
*
* \brief	interfaces for Wels Ultiliy lib
*
* \date	4/24/2012  Created by Derrick Jin
*
*************************************************************************************
*/

#include "WelsCommonType.h"

#include "WelsLock.h"

#define PURE = 0

typedef struct Wels_IID{
    uint32_t  Data1;
    uint16_t  Data2;
    uint16_t  Data3;
    uint8_t   Data4[8];
}WelsIID;

#define IsEqualWelsIID(iid1,iid2)  (!memcmp(&iid1,&iid2,sizeof(WelsIID)))


static const WelsIID WelsIID_IUnknown = 
{ 0x00000000, (uint16_t)(0x7819), (uint16_t)(0x2510), { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef } };

#ifdef WIN32

#define WELS_NOVTABLE __declspec(novtable)


#else

#define WELS_NOVTABLE

#endif


namespace WelsUtilLib{

class WELS_NOVTABLE IWelsUnknown
{
public:
    virtual long AddRef()PURE;
    virtual long Release()PURE;
    virtual long QueryInterface(const WelsIID& iid,void** vvp)PURE;

    virtual ~IWelsUnknown(){}
};

class CWelsUnknown:public IWelsUnknown
{
public:
    CWelsUnknown():m_lRef(0)
    {

    }
    virtual ~CWelsUnknown()
    {

    }

    virtual long AddRef()
    {
#ifdef WIN32
        InterlockedIncrement( (volatile long *)(&m_lRef) );
#else
        m_clock.Lock();
        ++m_lRef;
        m_clock.Unlock();
#endif
        return m_lRef;
    }

    virtual long Release()
    {
#ifdef WIN32
        InterlockedDecrement( (volatile long *)(&m_lRef) );  
#else     
        m_clock.Lock();
        --m_lRef;
        m_clock.Unlock();
#endif
        if (m_lRef==0){
            delete this;
            return 0;
        }
        return m_lRef;
    }

    virtual long QueryInterface(const WelsIID& iid,void** ppv)
    {
        if (ppv == NULL){
            return WELS_E_POINTER;
        }
   
       if (IsEqualWelsIID(iid,WelsIID_IUnknown)){
           *ppv = static_cast<IWelsUnknown*>(this);
       }else{
            *ppv = NULL;
            return WELS_E_NOINTERFACE;
       }

       (static_cast<IWelsUnknown*>(this))->AddRef();
       
       return WELS_S_OK;
    }
protected:
    volatile long m_lRef;
#ifndef WIN32
    CWelsLock m_clock;
#endif
};



}///namespace WelsUtilLib



