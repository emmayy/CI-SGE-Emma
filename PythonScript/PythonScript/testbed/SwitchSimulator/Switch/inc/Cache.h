#pragma once
#include "CacheItem.h"
#include "ICacheInterface.h"
#include <map>
#include <list>
/* fecth frame and packet index*/
static const unsigned char INVALID_PRIORITY  = 64;
static const int ONE_FETCH_COUNT = 40;
class  CSubscribeInfo 
{
	
public:
	CSubscribeInfo();
	
	virtual ~CSubscribeInfo();
public:
	//DWORD m_dwUserID;
	int m_nFrameIndex;
	int m_nPacketIndex;
	unsigned char m_bPriority;
	unsigned char m_bLastPriority;
	unsigned char m_bFetch;
	
	unsigned int  m_nLastSendSize;
	unsigned int  m_nPeriodSendSize;
	int m_nSndCnt;

};



class SwitchCache : public ISwitchCache
{
public:
	SwitchCache(unsigned long ulOwnerID,unsigned int nSessionID,unsigned int  nChannelID);
	~SwitchCache(void);

	//interface ISwitchCache
	virtual int OnSendCacheData();
	
	virtual int OnReceiveCacheData(unsigned char *pData, int dwDataLen, unsigned int ulTimeStamp, unsigned char bPriority,unsigned short uSeq);

	virtual int OnSendDataResult(unsigned int nResult, DWORD nUserID, int nFrameIndex, int nPacketIndex);

	virtual int OnUserSubscribe(DWORD nUserID, bool bSubscribe);

	//set the this cache's sink
	void OpenWithSink(ISwitchCacheSink *pSink);

private:
	bool IsCacheFull(int dwDataLen);

	unsigned long GetCacheUsedSize();

	unsigned long GetCacheItemUsedSize();
	

	bool RefreshCache(unsigned int ulTimeStamp,unsigned char bPriority);

	void CheckPriority(unsigned char bPriority);

	void UpdateSubscribeFrameIndex(int nCurFrameIndex,DWORD nUserID);

	void UpdateCacheIndex(int nFrameIndex);

	int SendCacheData(DWORD nUserID,CSubscribeInfo &subInfo);

	void EraseItemList(unsigned int nTimeStamp);

	bool CheckFrameIsUsed(int nFrameIndex);

	bool RefreshFrame(unsigned int ulTimeStamp,unsigned char bPriority);

	CacheFrame *GetFrameByIndex(int nIndex);

	bool UpdateSubscribeInfo(CSubscribeInfo &subInfo, CacheFrame *frame, DWORD nUserID,bool bDelete);

	void UpdateFrameWhenComeNew(CacheFrame *frame);
	
	int GetMaxPriorityFrameAfterIndex(int nFrameIndex);

	void UpdateSubscriberFrameByID(DWORD nUserID);

	void SendReceiverFeedBack(long long int nowTime);

	void UpdateReceiverBwFeedback(unsigned int nSendLen);

	int BeginSendOut(CacheFrame *frame,DWORD nUserID,CSubscribeInfo &subInfo, int nFrameIndex, int nPacketIndex);

	void UpdateUserFrameIndex(unsigned char bPriority);
private:
	//the cache item list
	typedef std::list<CacheFrame*> CACHE_FRAME_TYPE_LIST;
	CACHE_FRAME_TYPE_LIST  m_FrameList;
	//the userid fetch item and packet index map
	typedef std::map<DWORD, CSubscribeInfo> SUB_INFO_MAP_TYPE;
	SUB_INFO_MAP_TYPE m_SubscribeMap;

	typedef std::map<unsigned short, unsigned short> RECV_RANGER_MAP_TYPE;
	RECV_RANGER_MAP_TYPE   m_RecvRangerMap;
	int  m_nCurKeyIndex;  //latest key frame index
	int   m_nCurIndex;	  //the lastest frame index
	//the incomestream interface pointer
	ISwitchCacheSink	*m_pSink;

	CacheFrame *m_pFrame;

	unsigned long m_ulOwnerID;
	unsigned int  m_nSessionID;
	unsigned int  m_nChannelID;
	long long int m_ulLastFeedback;
	
};


