


#include <assert.h>
#include "WelsTrace.h"
#include "WelsLock.h"
#include "Cache.h"
#include "CacheManager.h"


using namespace WelsUtilLib;

static const unsigned long MAX_CACHE_SIZE = 1024*1024*20;  //the cache max size
static const unsigned int MAX_PACKET_SIZE = 1024*2;

static const unsigned char PACKET_PRIORITY_IDR = 0;      //the highest priority
static const unsigned char PACKET_PRIORITY_LDR = 2;		//the second highest priority
static const unsigned long FEEDBACK_PERIOD = 1000;



CSubscribeInfo::CSubscribeInfo()
{
		//m_dwUserID = 0;
		m_nFrameIndex = 0;
		m_nPacketIndex = 0;
		m_bPriority = INVALID_PRIORITY;
		m_bFetch  = 0;
		m_nSndCnt = ONE_FETCH_COUNT;
		m_bLastPriority = INVALID_PRIORITY;

		m_nPeriodSendSize = 0;
		m_nLastSendSize = 0;
		
}
CSubscribeInfo::~CSubscribeInfo()
{
}


//#define LOG_ELAPSE_TIME
SwitchCache::SwitchCache(unsigned long ulOwnerID,unsigned int nSessionID,unsigned int  nChannelID) 
	: m_nCurKeyIndex(-1)
	, m_nCurIndex(0)
	, m_pSink(NULL)
	, m_pFrame(NULL)
	, m_ulOwnerID(ulOwnerID)
	, m_nSessionID(nSessionID)
	, m_nChannelID(nChannelID)
	, m_ulLastFeedback(0)
{
	WELS_INFO_TRACE("SwitchCache::SwitchCache ownerid=" << ulOwnerID << ", " 
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << this);
}

SwitchCache::~SwitchCache(void)
{
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	
	for(; it != m_SubscribeMap.end();)
	{
		m_SubscribeMap.erase(it++);
	}
	m_SubscribeMap.clear();

	CACHE_FRAME_TYPE_LIST::iterator itFrame = m_FrameList.begin();
	for( ; itFrame != m_FrameList.end();)
	{
		CacheFrame *frame = *itFrame;
		
		m_FrameList.erase(itFrame++);
		if(frame != NULL)
		{
			delete frame;
			frame = NULL;
		}
		continue;
	}

	m_FrameList.clear();
	m_RecvRangerMap.clear();
	WELS_INFO_TRACE("SwitchCache::~SwitchCache" << ", " << this);
}

//send data to the subscribes
int SwitchCache::OnSendCacheData()
{
	//WELS_INFO_TRACE("SwitchCache::OnSendCacheData size = " << m_SubscribeMap.size());

	long long int nowTime = CWelsOSAbstract::WelsNow()/1000;

	if(m_ulLastFeedback == 0)
		m_ulLastFeedback = nowTime;

	if(nowTime - m_ulLastFeedback >= FEEDBACK_PERIOD)
	{
		SendReceiverFeedBack(nowTime);
		m_ulLastFeedback = nowTime;
	}
	
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	for(; it != m_SubscribeMap.end(); it++)
	{
		if(m_ulOwnerID == it->first)
			continue;
		CSubscribeInfo &subInfo = it->second;
		SendCacheData(it->first,subInfo);
	}
	
	return 0;
}


void SwitchCache::SendReceiverFeedBack(long long int nowTime)
{
	m_RecvRangerMap.clear();
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	for(; it != m_SubscribeMap.end(); it++)
	{
		if(m_ulOwnerID == it->first)
			continue;
		CSubscribeInfo &subInfo = it->second;
		float fv = float(subInfo.m_nPeriodSendSize)/(nowTime-m_ulLastFeedback);
		unsigned int v = (unsigned int)fv*8;
		UpdateReceiverBwFeedback(v);
		subInfo.m_nPeriodSendSize = subInfo.m_nLastSendSize = 0;
	}

	//
	if(m_SubscribeMap.size() > 0 && m_pSink)
	{
		//m_pSink->SendAsReceiveBwFeedback(m_RecvRangerMap.size(), m_RecvRangerMap);
	}
}


void SwitchCache::UpdateReceiverBwFeedback(unsigned int nSendLen)
{
	unsigned short ranger = nSendLen/100 + 1;
	RECV_RANGER_MAP_TYPE::iterator it = m_RecvRangerMap.find(ranger);
	if(it != m_RecvRangerMap.end())
	{
		unsigned short &uCnt = it->second;
		uCnt++;
	}
	else
	{
		m_RecvRangerMap[ranger] = 1;
	}
	
}

/**********************************************************************************************
*  Function: OnReceiveCacheData
*  Desc: receive the cache data from incoming stream, then cace the data
			
		
*  Parameters : 
			pData ;			the data which should cache
			dwDataLen;		cache data size
			ulTimeStamp;	incoming data timestamp
			nPacketIndex;	incoming data priority
*  Return : zero is succeed,otherwhise failure
*************************************************************************************************/

//receive the cache data from incoming stream; 
//the send data is point to current cache's nFrameIndex frame's
//nPacketIndex rtp data
CacheFrame* SwitchCache::GetFrameByIndex(int nIndex)
{
	CacheFrame *frame = NULL;
	if(nIndex >= m_FrameList.size())
		return frame;
	int index = 0;
	CACHE_FRAME_TYPE_LIST::iterator itFrame = m_FrameList.begin();
	for( ; itFrame != m_FrameList.end(); itFrame++)
	{
		if(index == nIndex)
			return *itFrame;
		index++;
	}
	
	WELS_INFO_TRACE("SwitchCache::GetFrameByIndex index=" << nIndex << " is return null" 
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
	return frame;
}
int SwitchCache::OnReceiveCacheData(unsigned char *pData, int dwDataLen, unsigned int ulTimeStamp, unsigned char bPriority,unsigned short uSeq)
{
	//check the cache size,if the cache size reach the max cache size,firt
	//DWORD tStart = GetTickCount();

	if(IsCacheFull(dwDataLen))
	{
		if(!RefreshCache(ulTimeStamp,bPriority))
			return -1;
	}
	
	bool bNew = false;
	
	CacheFrame *frame = m_pFrame;
	if(frame != NULL)
	{
		// if add the packet to exist frame needn't check the priority
		if(frame->GetTimeStamp() == ulTimeStamp)
		{
			if(uSeq == frame->GetLastSeq())
				return -1;
			WELS_INFO_TRACE("SwitchCache::OnReceiveCacheData packet seq=" << uSeq
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			return frame->AddCacheData(pData,dwDataLen,uSeq);
		}
		else
		{
			bNew = true;
			frame->SetItemCompletedFlag(1);
			WELS_INFO_TRACE("SwitchCache::OnReceiveCacheData packetcnt=" << frame->GetPacketCount()
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			UpdateFrameWhenComeNew(frame);
		}
	}
	else
	{
			
		bNew = true;
	}
	
	if(bNew)
	{
		//more than tow frame should check priority
		if(m_FrameList.size() >= 1)
			CheckPriority(bPriority);
		//
		WELS_INFO_TRACE("SwitchCache::OnReceiveCacheData cache size=" << GetCacheItemUsedSize()
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		CacheFrame *frame = new CacheFrame(ulTimeStamp,bPriority);
		frame->AddCacheData(pData,dwDataLen,uSeq);
		m_FrameList.push_back(frame);
		m_nCurIndex = m_FrameList.size()-1;
		m_pFrame = frame;
		//
		if(bPriority == PACKET_PRIORITY_IDR)
			m_nCurKeyIndex = m_nCurIndex;
		
		UpdateUserFrameIndex(bPriority);
	}
	
	return 0;
}



/**********************************************************************************************
*  Function: OnSendDataResult
*  Desc: receive the send result nofity which send to the user whose id is uUserID ; 
		 the send data is point to current cache's nFrameIndex frame's
		 nPacketIndex rtp data
			
		
*  Parameters : 
			nResult ;		indicate send result; zero represent successful, otherwhise failure
			nUserID;		indicate a subscribe user
			nFrameIndex;	indicate which  frame, must more or equal than zero
			nPacketIndex;	indicate which rtp packet,must more or equal than zero
*  Return :zero is succeed,otherwhise failure
*************************************************************************************************/

//receive the send result nofity which send to the user whose id is uUserID ; 
//the send data is point to current cache's nFrameIndex frame's
//nPacketIndex rtp data

int SwitchCache::OnSendDataResult(unsigned int nResult, DWORD nUserID, int nFrameIndex, int nPacketIndex)
{
	if(nResult != 0)
		return -1;
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.find(nUserID);
	if(it == m_SubscribeMap.end())
	{
		
		WELS_ERROR_TRACE("SwitchCache::OnSendDataResult UserID=" << nUserID << " not exist" 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	
	CSubscribeInfo &subInfo = it->second;
	subInfo.m_nPeriodSendSize += subInfo.m_nLastSendSize;

	int size = m_FrameList.size();
	
	//assert(nFrameIndex < size);
	if(nFrameIndex >= size)
	{
		
		WELS_ERROR_TRACE("SwitchCache::OnSendDataResult Frameindex=" << nFrameIndex <<  " nUserID=" << nUserID << " is invalid," 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	bool bDel = false;
	CacheFrame *frame = GetFrameByIndex(nFrameIndex);
	if(frame == NULL)
	{
		
		WELS_ERROR_TRACE("SwitchCache::OnSendDataResult Frameindex=" << nFrameIndex <<  " nUserID=" << nUserID << " is return null," 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	
	//assert(nPacketIndex < frame->GetPacketCount());
	if(nPacketIndex >= frame->GetPacketCount())
	{
		WELS_ERROR_TRACE("SwitchCache::OnSendDataResult PacketIndex=" << nPacketIndex <<  " nUserID=" << nUserID 
			<< ",nFrameIndex=" << nFrameIndex << " is invalid," 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	
	bool bUpdate = false;
	int nNextFrame = nFrameIndex;
	//check whether the user have completed send  nFrameIndex frame data successfully
	//if successfully then set next send frame index and packet index
	if((nPacketIndex + 1) == frame->GetPacketCount() && frame->GetItemCompletedFlag() == 1)
	{
		bUpdate = true;
		int nRef = frame->SubReferenceCount();
		//CLogFile::Instance()->WriteLog(LOG_INFO,"SwitchCache::OnSendDataResult goto next frame Frameindex=%d ,nUserID=%d,ref=%d ",nFrameIndex,nUserID,nRef);
		WELS_INFO_TRACE("SwitchCache::OnSendDataResult goto next Frameindex=" << nFrameIndex <<  " nUserID=" << nUserID 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		//check the nFrameIndex frame can be delete from cache
		if(nRef <= 0 && frame->GetPriority() > PACKET_PRIORITY_LDR)
		{
			WELS_INFO_TRACE("SwitchCache::OnSendDataResult del frame at Frameindex=" << nFrameIndex <<  " nUserID=" << nUserID  
				<< "," << " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			bDel = true;
		}
		
	}
	
	
	if(!bDel && !bUpdate)
	{
		//update the userid frame and packet index
		/*int nNextPacket = nPacketIndex + 1;
		subInfo.m_nFrameIndex = nNextFrame;
		subInfo.m_nPacketIndex = nNextPacket;*/
		WELS_INFO_TRACE("SwitchCache::OnSendDataResult Frameindex=" << nFrameIndex <<  ",nPacketIndex=" << nPacketIndex
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
	}
	else
	{
		UpdateSubscribeInfo(subInfo,frame,it->first,bDel);
	}
	
	return 0;
}
//update the last frame when receive new frame;
//when the user whose next packet index greater than the parameter's
// packet index and the user's next frame index equal than current frame index
void SwitchCache::UpdateFrameWhenComeNew(CacheFrame *frame)
{
#ifdef LOG_ELAPSE_TIME
	timeval tvStart;
	::gettimeofday(&tvStart, NULL);
#endif

	int nPacket = frame->GetPacketCount();
	unsigned char bPriority = frame->GetPriority();
	std::vector<int> m_vecID;
	bool bFlag = false;
	int nRef = 100;
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	for(;it != m_SubscribeMap.end();it++)
	{
		CSubscribeInfo &subInfo = it->second;
		
		if(m_nCurIndex < subInfo.m_nFrameIndex || (subInfo.m_nPacketIndex>= nPacket && m_nCurIndex == subInfo.m_nFrameIndex))
		{
			bFlag  = true;
			subInfo.m_nFrameIndex = m_nCurIndex;
			
			subInfo.m_nPacketIndex = 0;
			if(subInfo.m_bFetch)
			{
				nRef = frame->SubReferenceCount();
				WELS_INFO_TRACE("SwitchCache::UpdateFrameWhenComeNew decrease reference  at Frameindex=" << m_nCurIndex 
					<<  ",nUserID=" << it->first << ",ref=" << nRef << "," 
					<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			}
			
			subInfo.m_bFetch = 0;
			
			m_vecID.push_back(it->first);
		}
	}

	bool bDel = false;
	if(bFlag)
	{
		if(nRef <= 0 && frame->GetPriority() > PACKET_PRIORITY_LDR)
		{
			EraseItemList(frame->GetTimeStamp());
			WELS_INFO_TRACE("SwitchCache::UpdateFrameWhenComeNew priority=" << frame->GetPriority() 
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			delete frame;
			bDel = true;
			//CLogFile::Instance()->WriteLog(LOG_INFO,"SwitchCache::UpdateFrameWhenComeNew del  at Frameindex=%d",m_nCurIndex);
		}
	}
	//
	if(!bDel)
	{
		for(int i = 0; i < m_vecID.size(); i++)
		{
			UpdateSubscriberFrameByID(m_vecID.at(i));
		}
	}

	WELS_INFO_TRACE("SwitchCache::UpdateFrameWhenComeNew end," 
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
#ifdef LOG_ELAPSE_TIME
	long nElaped = 0;
	timeval tvEnd;
	
	::gettimeofday(&tvEnd, NULL);
	long nSec = tvEnd.tv_sec - tvStart.tv_sec;
	nElaped = nSec*CM_ONE_SECOND_IN_USECS + tvEnd.tv_usec - tvStart.tv_usec;
	WELS_INFO_TRACE("SwitchCache::UpdateSubscribeInfo time diff=" << nElaped << "," << this);
#endif

}

void SwitchCache::UpdateSubscriberFrameByID(DWORD nUserID)
{
	//CLogFile::Instance()->WriteLog(LOG_INFO,"SwitchCache::UpdateSubscriberFrameByID decrease reference  at nUserID=%d",nUserID);
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.find(nUserID);
	if(it != m_SubscribeMap.end())
	{
		
		CSubscribeInfo &subInfo = it->second;
		subInfo.m_nFrameIndex++;		
	}
}

bool SwitchCache::UpdateSubscribeInfo(CSubscribeInfo &subInfo,CacheFrame *frame,DWORD nUserID,bool bDelete)
{
	if(frame == NULL)
	{
		WELS_INFO_TRACE("SwitchCache::UpdateSubscribeInfo frame is null," << this);
		return false;
	}
#ifdef LOG_ELAPSE_TIME
	timeval tvStart;
	::gettimeofday(&tvStart, NULL);
#endif
	unsigned char bPrioirity = subInfo.m_bPriority;
	//subInfo.m_bLastPriority = bPrioirity;
	int nFrameIndex = subInfo.m_nFrameIndex;
	int nNextFrameIndex = -1;
	bool bFind = false;
	int nCount = m_FrameList.size()-nFrameIndex-1;
	WELS_INFO_TRACE("SwitchCache::UpdateSubscribeInfo frameindex=" << nFrameIndex << ",priority=" 
		<< bPrioirity << ",last=" << nCount
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this );
	int index = 0;
	if(bPrioirity == PACKET_PRIORITY_IDR)
	{
		CACHE_FRAME_TYPE_LIST::reverse_iterator itR = m_FrameList.rbegin();
		for(; itR != m_FrameList.rend();)
		{
			if(nCount <= 0)
			{
				break;
			}

			CacheFrame *frame = *itR;
			if(frame == NULL){
				itR++;
				index++;
				nCount--;
				continue;
			}

			if(frame->GetPriority() == bPrioirity)
			{
				nNextFrameIndex = m_FrameList.size() - index -1;
				bFind = true;
				break;
			}
			
			index++;
			nCount--;
			
			itR++;
		}

		if(!bFind)
		{
			//
			nNextFrameIndex = GetMaxPriorityFrameAfterIndex(nFrameIndex);
			if(nNextFrameIndex >= 0)
				bFind = true;
		}

	}
	else
	{
		/*CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
		for(; it != m_FrameList.end(); it++)
		{
			
			if(index <= nFrameIndex)
			{
				index++;
				continue;
			}

			CacheFrame *frame = *it;
			if(frame == NULL){
				index++;
				continue;
			}

			if(frame->GetPriority() < bPrioirity)
			{
				nNextFrameIndex = index;
				bFind = true;
				break;
			}
			index++;
		}*/

		nNextFrameIndex = GetMaxPriorityFrameAfterIndex(nFrameIndex);
		if(nNextFrameIndex >= 0)
			bFind = true;
		
	}

	
	if(!bFind)
	{
		nNextFrameIndex = nFrameIndex + 1;
		if(nNextFrameIndex > m_FrameList.size())
			nNextFrameIndex = m_FrameList.size();
	}

	subInfo.m_nFrameIndex = nNextFrameIndex;
	
	
	WELS_INFO_TRACE("SwitchCache::UpdateSubscribeInfo the next frame after nFrameIndex=" 
		<< nFrameIndex << ",next frame= " << nNextFrameIndex
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
	//
	subInfo.m_nPacketIndex = 0;
	subInfo.m_bFetch = 0;
	//
	//
	if(bDelete)
	{
		EraseItemList(frame->GetTimeStamp());
		delete frame;
		UpdateSubscribeFrameIndex(nFrameIndex,nUserID);
		UpdateCacheIndex(nFrameIndex);
		subInfo.m_nFrameIndex--;
	}
	
#ifdef LOG_ELAPSE_TIME
	long nElaped = 0;
	timeval tvEnd;
	
	::gettimeofday(&tvEnd, NULL);
	long nSec = tvEnd.tv_sec - tvStart.tv_sec;
	nElaped = nSec*CM_ONE_SECOND_IN_USECS + tvEnd.tv_usec - tvStart.tv_usec;
	WELS_INFO_TRACE("SwitchCache::UpdateSubscribeInfo time diff=" << nElaped << "," << this);
#endif	
	
	return nNextFrameIndex >=0 ? true : false;
}

int SwitchCache::GetMaxPriorityFrameAfterIndex(int nFrameIndex)
{
	int nNextFrameIndex = -1;
	int index = 0;
	unsigned char bPriority = INVALID_PRIORITY;
	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end(); it++)
	{
		if(index <= nFrameIndex)
		{
			index++;
			continue;
		}
		else
		{
			CacheFrame *frame = *it;
			if(frame == NULL)
			{
				index++;
				continue;
			}

			if(frame->GetPriority() == PACKET_PRIORITY_IDR)
			{
				return index;
			}
			if(frame->GetPriority() < bPriority)
			{
				bPriority = frame->GetPriority();
				nNextFrameIndex = index;
			}
			index++;
		}

	}
	return nNextFrameIndex;
}
/**********************************************************************************************
*  Function: OnUserSubscribe
*  Desc: receive the subscribe/unsubscribe information ; 
		
*  Parameters : 
			nUserID;		indicate a subscribe/unsubscribe 
			bSubscribe;		if true subscribe else unsubscribe
*  Return :zero is succeed,otherwhise failure
*************************************************************************************************/

//receive the subscribe/unsubscribe information ; 

int SwitchCache::OnUserSubscribe(DWORD nUserID, bool bSubscribe)
{
	bool bDel = false;
	int ret = -1;
	WELS_INFO_TRACE("SwitchCache::OnUserSubscribe user id=" << nUserID << ",sub=" << bSubscribe
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
	if(bSubscribe)
	{
		SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.find(nUserID);
		if(it != m_SubscribeMap.end())
		{
			WELS_ERROR_TRACE("SwitchCache::OnUserSubscribe user aready exist id=" << nUserID 
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
			return ret;
		}
		CSubscribeInfo subInfo;
		if(m_nCurKeyIndex > 0)
			subInfo.m_nFrameIndex = m_nCurKeyIndex;

		m_SubscribeMap[nUserID] = subInfo;
	}
	else
	{
		SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.find(nUserID);
		if(it != m_SubscribeMap.end())
		{
			int size = m_FrameList.size();
			//assert(it->second.m_nFrameIndex < size);
			if(it->second.m_nFrameIndex >= size)
			{
				int index = it->second.m_nFrameIndex;
				WELS_ERROR_TRACE("SwitchCache::OnUserSubscribe invalid frameindex=" << index 
					<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
				return ret;
			}
			int nFrameIndex = -1;
			nFrameIndex = it->second.m_nFrameIndex;
			//if the user have get data at nFrameIndex frame's nPacketIndex rtp data
			//then should decrease ref count
			if(it->second.m_bFetch)
			{
				CacheFrame *frame = GetFrameByIndex(nFrameIndex);
				if(frame != NULL)
				{
					
					unsigned char bPriority = frame->GetPriority();
					//if the frame can delete then delete it
					if(frame->SubReferenceCount() <= 0 && (bPriority > PACKET_PRIORITY_LDR))
					{
						EraseItemList(frame->GetTimeStamp());
						delete frame;
						frame = NULL;
						bDel = true;
					}

				}
			}
			m_SubscribeMap.erase(it);
			if(bDel)
			{
				UpdateSubscribeFrameIndex(nFrameIndex,0);
				UpdateCacheIndex(nFrameIndex);
			}
		}
		else
		{
			WELS_ERROR_TRACE("SwitchCache::OnUserSubscribe user not exist id=" << nUserID 
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		}
	}

	return 0;
}

void SwitchCache::OpenWithSink(ISwitchCacheSink *pSink)
{
	m_pSink = pSink;
}
//update the cache subscribe's next get frame and packet index
void SwitchCache::UpdateSubscribeFrameIndex(int nCurFrameIndex,DWORD nUserID)
{
	//assert( 0=<nFrameIndex && nFrameIndex <= m_ItemVec.size());
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	
	for(; it != m_SubscribeMap.end(); it++)
	{
		if(it->first == nUserID)
			continue;
		CSubscribeInfo &subInfo = it->second;
		if(subInfo.m_nFrameIndex >= nCurFrameIndex)
		{
			subInfo.m_nFrameIndex-=1;
			
		}
	}
}
//remove the frame which timestamp is equal nTimeStramp 
//from the cache
void SwitchCache::EraseItemList(unsigned int nTimeStamp)
{
	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end(); it++)
	{
		CacheFrame *frame = *it;
		if(frame == NULL)
			continue;
		if(frame->GetTimeStamp() == nTimeStamp)
		{
			m_FrameList.erase(it);
			break;
		}
	}
}

//update the latest frame and key frame index

void SwitchCache::UpdateCacheIndex(int nFrameIndex)
{
	if(m_nCurKeyIndex >= nFrameIndex)
		m_nCurKeyIndex--;
	m_nCurIndex = m_FrameList.size()-1;
}

int SwitchCache::SendCacheData(DWORD nUserID,CSubscribeInfo &subInfo)
{
	if(m_pSink == NULL)
	{
		WELS_INFO_TRACE("SwitchCache::SendCacheData m_pSink is null" << this);
		return -1;
	}
	
	int nFrameIndex = -1;
	nFrameIndex = subInfo.m_nFrameIndex;
	
	int size = m_FrameList.size();
	if(nFrameIndex >= size)
	{
		WELS_INFO_TRACE("SwitchCache::SendCacheData nFrameIndex=" << nFrameIndex << "userid = " << nUserID << " index not exist" 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}

	if(nFrameIndex < 0)
		return -1;

	CacheFrame *frame = GetFrameByIndex(nFrameIndex);
	if(frame == NULL)
	{
		WELS_INFO_TRACE("SwitchCache::SendCacheData nFrameIndex=" << nFrameIndex << "userid = " << nUserID << " frame not exist" 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	
	int nPacketIndex = subInfo.m_nPacketIndex;
	int nPacketCount = frame->GetPacketCount();
	if(nPacketIndex >= nPacketCount)
	{
		WELS_INFO_TRACE("SwitchCache::SendCacheData nFrameIndex=" << nFrameIndex << "userid = " << nUserID << "packetindex=" << nPacketIndex << " index not exist" 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}

	//if get first packet set the frame's priority to this user
	if(nPacketIndex == 0)
	{
		if(!subInfo.m_bFetch)
		{
			subInfo.m_bPriority = frame->GetPriority();
			subInfo.m_bFetch = 1;
			frame->AddReferenceCount();
			int nRef = frame->GetReferenceCount();
			WELS_INFO_TRACE("SwitchCache::SendCacheData first get frameindex=" << nFrameIndex << "userid = " << nUserID
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		}
			
	}	

	int nSucceed = 0;
	//send a group new rtp packet 
	int nNewsend = subInfo.m_nSndCnt;
	if(nPacketCount < (nPacketIndex+nNewsend))
	{
		nNewsend = nPacketCount - nPacketIndex;
	}

	WELS_INFO_TRACE("SwitchCache::SendCacheData start frame = "<< nFrameIndex <<",packet = " << nPacketIndex
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID  << "," << this);
	
	if(nNewsend > 0)
	{
		int index = -1;
		for(int i = 0; i < nNewsend; i++)
		{
			index = nPacketIndex+i;
			//if send return failed, then break loop
			if(BeginSendOut(frame,nUserID,subInfo,nFrameIndex,index) != 0)
			{
				break;
			}
			
			nSucceed++;
			
		}
		//adjust next get packet index 
		if(subInfo.m_bFetch)
			subInfo.m_nPacketIndex += nSucceed;
	}
	WELS_INFO_TRACE("SwitchCache::SendCacheData nFrameIndex=" << nFrameIndex << "userid = " << nUserID << "packetindex=" << nPacketIndex << 
		",nSucceed=" << nSucceed << ",nNewsend=" << nNewsend
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
	//CLogFile::Instance()->WriteLog(LOG_INFO,"SwitchCache::SendCacheData nUserID=%d,nFrameIndex = %d, nPacketIndex = %d  end",nUserID,nFrameIndex,nPacketIndex);
	
	//DWORD  tEnd = GetTickCount();
	//DWORD diff = tEnd - tStart;
	//CLogFile::Instance()->WriteLog(LOG_INFO,"SwitchCache::SendCacheData time tick=%d,nUserID=%d,nFrameIndex = %d, nPacketIndex = %d",diff,nUserID,nFrameIndex,nPacketIndex);
	return 0;
}

int SwitchCache::BeginSendOut(CacheFrame *frame,DWORD nUserID, CSubscribeInfo &subInfo, int nFrameIndex, int nPacketIndex)
{
	if(!frame) return -1;


	unsigned char RtpData[MAX_PACKET_SIZE] = {0};
	unsigned int  nDataLen = 0;

	frame->GetCacheData(RtpData,nDataLen,nPacketIndex);
	if(nDataLen == 0)
	{
		WELS_ERROR_TRACE("SwitchCache::BeginSendOut nPacketIndex=" << nPacketIndex << " get rtp data len = 0" 
			<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return -1;
	}
	/*unsigned short seq = CWseRtpPacket::get_sequence_number((BYTE*)RtpData);
	
	WELS_INFO_TRACE("SwitchCache::BeginSendOut nFrameIndex=" << nFrameIndex << " userid = " << nUserID << " packetindex=" << nPacketIndex 
		<< "seq = " << seq << " timestamp = " << frame->GetTimeStamp()
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this); */
	subInfo.m_nLastSendSize = nDataLen;
	return m_pSink->OnSendOutCacheData(RtpData,nDataLen,nUserID,nFrameIndex,nPacketIndex, frame->GetTimeStamp());
}
//check the cahce size reach max cache size
//if reached return true;else return false
bool SwitchCache::IsCacheFull(int dwDataLen)
{
	if((GetCacheUsedSize() + dwDataLen) >= MAX_CACHE_SIZE)
		return true;
	return false;
}
//get the cache used size 
unsigned long SwitchCache::GetCacheUsedSize()
{
	
	unsigned long ulSize = 0;
	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end(); it++)
	{
		CacheFrame *frame = *it;
		if(frame == NULL)
			continue;
		ulSize += frame->GetCacheItemSize();
	}

	return ulSize;
}

//get the cache used size 
unsigned long SwitchCache::GetCacheItemUsedSize()
{
	
	unsigned long ulSize = 0;
	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end(); it++)
	{
		CacheFrame *frame = *it;
		if(frame == NULL)
			continue;
		ulSize += frame->GetCacheBufferSize();
	}

	return ulSize;
}


// current not use this function
bool SwitchCache::CheckFrameIsUsed(int nFrameIndex)
{
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	for(; it != m_SubscribeMap.end(); it++)
	{
		CSubscribeInfo &subInfo = it->second;
		if(subInfo.m_nFrameIndex == nFrameIndex)
		{
			return true;
		}
	}
	return false;
}

bool SwitchCache::RefreshCache(unsigned int ulTimeStamp,unsigned char bPriority)
{
	//if all the frame's priority equal or lower than bPriority
	//should delete the frames before the latest key frame
	//no matter it is use or unuse
	//WELS_INFO_TRACE("SwitchCache::RefreshCache," << this);
	return RefreshFrame(ulTimeStamp,bPriority);
	
}

/*************************************************************************************************************
*  Function: RefreshFrame
*  Desc: delete cache frame when the cache is full;
		the flow first try to delete the frame which priority lower or equal than current receive
		data priority and it have not used by all subscribe users ; 
		if cann't find that frame then delete the frame which priority lower or equal than current
		receive data priority
		
*  Parameters : 
			ulTimeStamp;	current receive data timestamp noused
			bPriority;		current receive data priority  noused
			
*  Return :true is succeed,otherwhise failure
************************************************************************************************************/

bool SwitchCache::RefreshFrame(unsigned int ulTimeStamp,unsigned char bPriority)
{
#ifdef LOG_ELAPSE_TIME
	timeval tvStart;
	::gettimeofday(&tvStart, NULL);
#endif

	int index = 0;
	CACHE_FRAME_TYPE_LIST::reverse_iterator itR = m_FrameList.rbegin();
	for(; itR != m_FrameList.rend(); )
	{
		CacheFrame* frame = *itR;
		if(frame == NULL)
		{
			//m_FrameList.erase(itR++);
			itR++;
			index++;
			continue;
		}

		if(frame->GetPriority() == PACKET_PRIORITY_IDR)
			break;
		index++;
		itR++;
	}

	int nDel = m_FrameList.size() - index -1;
	
	if(nDel <= 0)
	{
		//the cache is full but the first frame is the only one key frame
		WELS_ERROR_TRACE("SwitchCache::RefreshFrame have only one ldr frame when cache is full," 
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);
		return false;
	}
	int nCurrent = 0;

	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end();)
	{

		if(nCurrent < nDel)
		{
			CacheFrame* frame = *it;
			if(frame != NULL)
			{
				delete frame;
				frame = NULL;
			}
			nCurrent++;
			m_FrameList.erase(it++);
			continue;
		}
		else
		{
			break;
		}
	}
	//update current key frame index
	m_nCurKeyIndex = 0;
	m_nCurIndex = m_FrameList.size()-1;
	if(m_nCurIndex < 0)
		m_nCurIndex = 0;
	//
	SUB_INFO_MAP_TYPE::iterator itSub = m_SubscribeMap.begin();
	for(; itSub != m_SubscribeMap.end(); itSub++)
	{
		CSubscribeInfo &subInfo = itSub->second;
		if(subInfo.m_nFrameIndex >= nDel )
		{
			subInfo.m_nFrameIndex-=nDel;	
		}
		else
		{
			subInfo.m_nFrameIndex = 0;
			subInfo.m_nPacketIndex = 0;
			subInfo.m_bFetch = 0;
		}
	}
#ifdef LOG_ELAPSE_TIME
	long nElaped = 0;
	timeval tvEnd;
		
	::gettimeofday(&tvEnd, NULL);
	long nSec = tvEnd.tv_sec - tvStart.tv_sec;
	nElaped = nSec*CM_ONE_SECOND_IN_USECS + tvEnd.tv_usec - tvStart.tv_usec;
	WELS_INFO_TRACE("SwitchCache::RefreshFrame time diff=" << nElaped << "," << this);
#endif
	WELS_INFO_TRACE("SwitchCache::RefreshFrame ulTimeStamp=" << ulTimeStamp << ",bPriority=" << bPriority 
	<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);

	return true;
}

//compare the cache's all frame with bProirity
//if the frame priority lower or equal than the give prority
//should update the frame reference; delete it if satisfy the delete condition
void SwitchCache::CheckPriority(unsigned char bPriority)
{
#ifdef LOG_ELAPSE_TIME
	timeval tvStart;
	::gettimeofday(&tvStart, NULL);
#endif
	int nFrameIndex = 0;
	int nDeleted = 0;
	
	CACHE_FRAME_TYPE_LIST::iterator it = m_FrameList.begin();
	for(; it != m_FrameList.end();)
	{
		bool bDelete = false;
		CacheFrame *frame = *it;
		if(frame == NULL)
		{
			m_FrameList.erase(it++);
			nFrameIndex++;
			continue;
		}
		unsigned char bCachedPriority  = frame->GetPriority();
		//(frame priority equal current receive priority and priority == 0) or current 
		//receive priority higher than frame priority  then notify the cache frame to sub
		//the reference count
		if(bCachedPriority == bPriority)
		{
			if(bPriority == PACKET_PRIORITY_IDR)
			{
				frame->ReceiveHighPriority();
			}
		}
		else if(bPriority < bCachedPriority)
		{
			frame->ReceiveHighPriority();
		}
		//delete the cache frame if reference reach zero
		if(frame->GetReferenceCount() <= 0)
		{
			
			bDelete = true;
			
		}
		
		nFrameIndex++;
		//update subscribe's next fetch frame index
		if(bDelete)
		{	
			WELS_INFO_TRACE("SwitchCache::CheckPriority timestamp=" << frame->GetTimeStamp() 
				<< " priority="<< frame->GetPriority() << " bPriority = " << bPriority
				<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID <<"," << this);
			delete frame;
			frame = NULL;

			nDeleted++;
			//update the subscriber's frameindex
			SUB_INFO_MAP_TYPE::iterator itSub = m_SubscribeMap.begin();
			for(; itSub!= m_SubscribeMap.end(); itSub++)
			{
				CSubscribeInfo &subInfo = itSub->second;
				if(subInfo.m_nFrameIndex > (nFrameIndex-nDeleted))
				{
					subInfo.m_nFrameIndex--;	
				}
			}
					//update latest input frame index;
			m_nCurIndex--;
			
			//update latest key frame index
			if((nFrameIndex-nDeleted) <  m_nCurKeyIndex)
				m_nCurKeyIndex--;
			m_FrameList.erase(it++);
			continue;
		}
		
		it++;
	}

	
	WELS_INFO_TRACE("SwitchCache::CheckPriority have frame size = " << m_FrameList.size()
		<< " nSessionID = " << m_nSessionID << " nChannelID = " << m_nChannelID << "," << this);

	CACHE_FRAME_TYPE_LIST::iterator itReminder = m_FrameList.begin();
	for(; itReminder != m_FrameList.end();)
	{
		CacheFrame *frame = *itReminder;
		if(frame == NULL)
		{
			itReminder++;
			continue;
		}
		WELS_INFO_TRACE("SwitchCache::CheckPriority have frame buf size = " << frame->GetCacheBufferSize()
			<< " timestamp = " << frame->GetTimeStamp() << " packet_cnt = " << frame->GetPacketCount());
		itReminder++;
	}
#ifdef LOG_ELAPSE_TIME
		long nElaped = 0;
		timeval tvEnd;
		
		::gettimeofday(&tvEnd, NULL);
		long nSec = tvEnd.tv_sec - tvStart.tv_sec;
		nElaped = nSec*CM_ONE_SECOND_IN_USECS + tvEnd.tv_usec - tvStart.tv_usec;
		WELS_INFO_TRACE("SwitchCache::CheckPriority time diff=" << nElaped << "," << this);
#endif
	
}


void SwitchCache::UpdateUserFrameIndex(unsigned char bPriority)
{
	SUB_INFO_MAP_TYPE::iterator it = m_SubscribeMap.begin();
	for(; it != m_SubscribeMap.end(); it++)
	{
		//if(m_dwOwnerID == it->first)
			//continue;
		CSubscribeInfo &subInfo = it->second;

		if(subInfo.m_nPacketIndex != 0)
			continue;
		INT nFrameIndex = subInfo.m_nFrameIndex;
		if( nFrameIndex < 0 && nFrameIndex >= m_FrameList.size()-1)
			continue;
		CacheFrame *frame = GetFrameByIndex(nFrameIndex);
		if(NULL == frame)
			continue;
		unsigned char aPriority = frame->GetPriority();
		if(aPriority == PACKET_PRIORITY_IDR || bPriority >= aPriority)
			continue;
		if(subInfo.m_bFetch)
			frame->SubReferenceCount();
		subInfo.m_bFetch = 0;
		subInfo.m_nFrameIndex = m_FrameList.size() - 1;
	}
}


 UINT  ulTimerID = 0;


VOID CALLBACK TimerProc(
    HWND hwnd,     // handle of window for timer messages
    UINT uMsg,     // WM_TIMER message
    UINT idEvent,  // timer identifier
    DWORD dwTime   // current system time
)
{	
	CCacheManager::Instance().SendCacheData();
}



extern "C" {

	SwitchCacheHandle   CreateSwitchHandle(ISwitchCacheSink * pSink)
	{
		SwitchCache  * pSwitchCache = new SwitchCache(-1, 0, 0);

		if( pSwitchCache ){
			pSwitchCache->OpenWithSink(pSink);

			CCacheManager::Instance().RegisterCache(pSwitchCache);
		}

		return pSwitchCache;
	}

	int                  Subscribe(SwitchCacheHandle  hSwitchCache, DWORD nUserID, bool bSubscribe)
	{
        ISwitchCache * pSwitchCache = (ISwitchCache*)hSwitchCache; 

		if( pSwitchCache ){
		    return	pSwitchCache->OnUserSubscribe(nUserID, bSubscribe);
		}
		return 0;
	}

	int                  ReceiveData(SwitchCacheHandle hSwitchCache, unsigned char * pData, int nLen, 
		unsigned int ulTimeStamp,  unsigned char bPriority,  unsigned short uSeq)
	{	
        ISwitchCache * pSwitchCache = (ISwitchCache*)hSwitchCache; 

		if( pSwitchCache ){
		    return	pSwitchCache->OnReceiveCacheData(pData, nLen, ulTimeStamp, bPriority, uSeq);
		}
		return 0;
	}

	int                  OnSendDataResult(SwitchCacheHandle hSwitchCache, unsigned int nResult, DWORD nUserID, int nFrameIndex, int nPacketIndex)
	{
		ISwitchCache * pSwitchCache = (ISwitchCache*)hSwitchCache; 

		if( pSwitchCache ){
			return pSwitchCache->OnSendDataResult(nResult, nUserID, nFrameIndex, nPacketIndex);
		}
		return 0;
	}

	//TODO:  should store the cache id 
	void                 DestroySwitchCache(SwitchCacheHandle hSwitchCache)
	{
		ISwitchCache * pSwitchCache = (ISwitchCache*)hSwitchCache; 

		if( pSwitchCache ){
            CCacheManager::Instance().UnRegisterCache(0);
		}
	}

	int                  StartCache()
	{
        ulTimerID = SetTimer(NULL, NULL, 20, NULL);

		return 0;
	}

	int                  StopCache()
	{
        KillTimer(NULL, ulTimerID);

		return 0;
	}
}


