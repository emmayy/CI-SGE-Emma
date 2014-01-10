
#include "WelsTrace.h"
#include "CacheItem.h"

//#include "WseDebug.h"
//#include "safe_mem_lib.h"

using namespace std;
CacheFrame::CacheFrame(unsigned int ulTimeStamp, unsigned char bPriority) : m_bPriority(bPriority)
	, m_dwRefCount(1)
	, m_ulTimeStamp(ulTimeStamp)
	, m_bRecvHighPriority(0)
	, m_ulDataLen(0)
	, m_uCacheBufLen(0)
	, m_bCompleted(0)
	, m_uLastSeq(0)
{
	WELS_INFO_TRACE("CacheFrame::CacheFrame"<< "ulTimeStamp=" << ulTimeStamp << ", bPriority=" << bPriority << "," << this);
}

CacheFrame::~CacheFrame(void)
{
	Destroy();
}

void CacheFrame::Destroy()
{
	CACHE_DATA_TYPE::iterator it = m_DataVec.begin();
	for(;it != m_DataVec.end();)
	{
		CacheData_t* data = NULL;
		data = *it;
		if(data == NULL)
		{
			m_DataVec.erase(it++);
			continue;
		}
		it = m_DataVec.erase(it);
		if(data->m_pData)
		{
			delete data->m_pData;
			data->m_pData = NULL;
		}
		
		delete data;
		data = NULL;
		
	}
	m_DataVec.clear();
}

// add a new rtp packet to the end of vector, and update the frame memory
//used size
int CacheFrame::AddCacheData(unsigned char* pData,unsigned int dwDataLen,unsigned short uSeq)
{
	CacheData_t* data = new CacheData_t();
	if(data == NULL)
	{
		WELS_ERROR_TRACE("SwitchCache::AddCacheData"<< "alloc mem failure, " << this);
		return -1;
	}
	
	data->m_pData = new unsigned char[dwDataLen +1];
	if(data->m_pData == NULL)
	{
	
		delete data;
		WELS_ERROR_TRACE("SwitchCache::AddCacheData"<< "alloc mem failure, " << this);
		return -1;
	}
	
	memcpy_s(data->m_pData,dwDataLen,pData,dwDataLen);
	//memcpy(data->m_pData,pData,dwDataLen);
	data->m_pData[dwDataLen] = '\0';
	data->m_dwDataLen = dwDataLen;

	m_DataVec.push_back(data);

	m_ulDataLen += sizeof(CacheData_t) + sizeof(unsigned int) + dwDataLen;
	m_uCacheBufLen += dwDataLen;
	m_uLastSeq = uSeq;
	return 0;
}
//get the frame's rtpdata which index equal parameter index 
int CacheFrame::GetCacheData(unsigned char* pData,unsigned int& dwDataLen, int index)
{
	if(!pData)  return -1;

	if(index >= m_DataVec.size())
	{
		WELS_ERROR_TRACE("SwitchCache::GetCacheData index=" << index << "is invalid, " << this);
		return -1;
	}
	
	CacheData_t* data = NULL;
	data = m_DataVec.at(index);
	if(data == NULL)
	{
		WELS_ERROR_TRACE("SwitchCache::GetCacheData"<< "is null, " << this);
		return -1;
	}

	memcpy_s(pData,data->m_dwDataLen,data->m_pData, data->m_dwDataLen);
	//memcpy(pData,data->m_pData, data->m_dwDataLen);
	dwDataLen = data->m_dwDataLen;

	return 0;
}
// when cache receive a new data which priorty equal or higher
//than this frame priority; this frame should decrease it reference
//that haven't decreased before
void CacheFrame::ReceiveHighPriority()
{
	if(m_bRecvHighPriority > 0)
		return;
	m_dwRefCount--;
	
	m_bRecvHighPriority = 1;
}

void CacheFrame::AddReferenceCount()
{
	m_dwRefCount++;
}
unsigned int CacheFrame::SubReferenceCount()
{
	m_dwRefCount--;
	return m_dwRefCount;
}
//get the frame memory used size
unsigned long CacheFrame::GetCacheItemSize()
{
	unsigned long ulSize = sizeof(unsigned long) + sizeof(unsigned int)*2
							+ sizeof(unsigned char)*3;
	
	ulSize += m_ulDataLen;
	
	return ulSize;
}

unsigned long CacheFrame::GetCacheBufferSize()
{
	return m_uCacheBufLen;
}
//get the frame data count
unsigned int CacheFrame::GetPacketCount()
{
	return m_DataVec.size();
}