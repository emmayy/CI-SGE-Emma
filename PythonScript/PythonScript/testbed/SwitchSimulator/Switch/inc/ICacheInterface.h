#pragma once





class ISwitchCacheSink //incoming stream should implement this interface
{
public:
	virtual int OnSendOutCacheData(unsigned char *pData, int dwDataLen,DWORD nUserID, int nFrameIndex, int nPacketIndex,
		unsigned int uiTimeStamp) = 0;

	virtual ~ISwitchCacheSink(){}
};

class ISwitchCache //switch_cache should implement this interface
{
public:
	virtual int OnSendCacheData() = 0;
	
	virtual int OnReceiveCacheData(unsigned char *pData, int dwDataLen, unsigned int ulTimeStamp, unsigned char bPriority,unsigned short uSeq) = 0;

	virtual int OnSendDataResult(unsigned int nResult, DWORD nUserID, int nFrameIndex, int nPacketIndex) = 0;

	virtual int OnUserSubscribe(DWORD nUserID, bool bSubscribe) = 0;

	virtual void OpenWithSink(ISwitchCacheSink *pSink) = 0;

	virtual ~ISwitchCache() {}
};


typedef void * SwitchCacheHandle;   

extern "C" {

    SwitchCacheHandle    CreateSwitchHandle(ISwitchCacheSink * pSink);
	int                  Subscribe(SwitchCacheHandle  hSwitchCache, DWORD nUserID, bool bSubscribe);
	int                  ReceiveData(SwitchCacheHandle hSwitchCache, unsigned char * pData, int nLen, 
		unsigned int ulTimeStamp,  unsigned char bPriority,  unsigned short uSeq);
	void                 DestroySwitchCache(SwitchCacheHandle hCache);
	int                  OnSendDataResult(SwitchCacheHandle hCache, unsigned int nResult, DWORD nUserID, int nFrameIndex, int nPacketIndex);

	int                  StartCache(); 
	int                  StopCache();
}

