#pragma once
#include <vector>

#define CLOCKS_PER_SEC 1000
//typedef long long _int64;
const long us_benchmark = 1000000; 
const long us_second = us_benchmark;//microseconds in one seconds
const long ms_benchmark = 1000;
const long ms_second = ms_benchmark;//milliseconds in one seconds

#define CM_ONE_SECOND_IN_USECS 1000000L


/*the CacheData_s struct contain a rpt packet data and datalen*/
typedef struct CacheData_s {
	unsigned char*	m_pData;
	unsigned int 	m_dwDataLen;

} CacheData_t;


class CacheFrame
{
public:
	CacheFrame(unsigned int ulTimeStamp, unsigned char bPriority);
	virtual ~CacheFrame(void);

	unsigned int GetTimeStamp() const { return m_ulTimeStamp; }
	unsigned char GetPriority() const { return m_bPriority; }
	unsigned int GetReferenceCount() const { return m_dwRefCount; }
	int AddCacheData(unsigned char* pData,unsigned int dwDataLen,unsigned short uSeq);
	int GetCacheData(unsigned char* pData,unsigned int& dwDataLen, int index);
	void ReceiveHighPriority();
	unsigned short GetLastSeq() const { return m_uLastSeq; }
	unsigned long GetCacheItemSize();
	unsigned long GetCacheBufferSize();
	void SetItemCompletedFlag(unsigned char bFlag) { m_bCompleted = bFlag; }
	unsigned char GetItemCompletedFlag() { return m_bCompleted; }
	unsigned int GetPacketCount();
	unsigned int SubReferenceCount();
	void AddReferenceCount();
private:
	void Destroy();
private:
	typedef std::vector<CacheData_t*> CACHE_DATA_TYPE;
	CACHE_DATA_TYPE  m_DataVec;

	unsigned int	m_ulTimeStamp;	// the frame timestamp
	unsigned long   m_ulDataLen;	//the frame data total length;
	unsigned long 	m_uCacheBufLen; // buffer total length
	unsigned int	m_dwRefCount;	//the frame reference
	unsigned char	m_bPriority;	// the frame priority
	unsigned char	m_bRecvHighPriority;//indicate the frame have decrease reference that the higher priority frame
											//data received;
	unsigned char   m_bCompleted;    //indicate the frame have receive all the data
	unsigned short  m_uLastSeq;
};
