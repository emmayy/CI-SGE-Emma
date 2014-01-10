

#ifndef _OUTGOING_H_
#define _OUTGOING_H_


#include <map>
#include "ICacheInterface.h"

#include "WelsOSAbstract.h"

using namespace WelsUtilLib;

class  CNetworkSink
{
public:
	enum {
		DEFAULT_BANDWIDTH = 200000,
	};
	CNetworkSink(int nBandwidth = DEFAULT_BANDWIDTH)
	{
		m_nBandwidth = nBandwidth;

		ulLastSendTimeStamp = 0;

		level = 0;
		m_nSendOutPacketCount = 0;
	}
	virtual ~CNetworkSink()
	{
		WELS_INFO_TRACE("NETWORKSINK: Total Send out Packet "<<m_nSendOutPacketCount);
	}

	// flow control 
    int  ReceiveData(unsigned char * pData, int nLen)
	{
		int  rc = 0;
		if( ulLastSendTimeStamp == 0 ){
			ulLastSendTimeStamp = CWelsOSAbstract::WelsNow();

			level = nLen * 8;
			++ m_nSendOutPacketCount;
			return 0;
		}

		unsigned int ulNow = CWelsOSAbstract::WelsNow();
		unsigned int ulDif; 

		if( ulNow >= ulLastSendTimeStamp ){
			ulDif = ulNow - ulLastSendTimeStamp;
		} else {
			ulDif = 0xfffffffful - ulLastSendTimeStamp + ulNow; 
		}

		ulLastSendTimeStamp = ulNow;
		if( level < m_nBandwidth ){
			level += nLen * 8;
			WELS_INFO_TRACE("NETWORKSINK:  send success, level = "<<level);
			++ m_nSendOutPacketCount;
			rc = 0;
		} else {
			WELS_INFO_TRACE("NETWORKSINK:  send failed, level = "<<level);
			rc = 1;
		}

		level -= ulDif * m_nBandwidth/1000;	

		if( level < 0 ){
			level = 0;
		}
		return rc;
	}


private:
	int  m_nBandwidth;	
	unsigned int ulLastSendTimeStamp;
	int  level;  
	int  m_nSendOutPacketCount;
};


typedef void (*DataDumpFunc)(unsigned char * pData, int nLen, unsigned int uiTimeStamp, FILE* pFile);

class  COutgoing : public ISwitchCacheSink
{
public:
	COutgoing(SwitchCacheHandle h = NULL)
	{
		m_hCache = h;
		m_fCallback = NULL;
	}

	virtual ~COutgoing()
	{
		std::map<DWORD, CNetworkSink*>::iterator it = m_cNetworkSinks.begin();

		while( it != m_cNetworkSinks.end() ){
			delete it->second;
			it ++;
		}

		m_cNetworkSinks.clear();
	}

	void  SetCache(SwitchCacheHandle h, DataDumpFunc  callback, FILE* pFile )
	{
		m_hCache = h;
		m_fCallback = callback;
		m_pFile = pFile;
	}	

	virtual int OnSendOutCacheData(unsigned char *pData, int dwDataLen,DWORD nUserID, int nFrameIndex, int nPacketIndex, 
		unsigned int uiTimeStamp)
	{
		std::map<DWORD, CNetworkSink*>::iterator it = m_cNetworkSinks.find(nUserID);

		if( it != m_cNetworkSinks.end() ){
			int result = it->second->ReceiveData(pData, dwDataLen);

			if( (result == 0) && (m_fCallback != NULL) ){
				m_fCallback(pData, dwDataLen, uiTimeStamp, m_pFile);
				//WELS_INFO_TRACE("Data go through,  frame idx "<<nFrameIndex<<", PacketIndex "<<nPacketIndex);
			}

			if( m_hCache ){
				OnSendDataResult(m_hCache, result, nUserID, nFrameIndex, nPacketIndex);
			}

			return result;
		}

		return 1;
	}

	void   Subscribe(DWORD nUserID, bool bSubscribe, int nBandwidth)
	{
		std::map<DWORD, CNetworkSink*>::iterator it = m_cNetworkSinks.find(nUserID);

		if( it != m_cNetworkSinks.end() ){
			if( !bSubscribe ){
				delete it->second; 
				m_cNetworkSinks.erase(it);
			}
		} else {
			if( bSubscribe) {
				m_cNetworkSinks[nUserID] = new CNetworkSink(nBandwidth);
			}
		}
	}

private:
	std::map<DWORD, CNetworkSink*>  m_cNetworkSinks;

	SwitchCacheHandle  m_hCache;
	DataDumpFunc       m_fCallback;
	FILE* m_pFile;

};





#endif
