
#include "WelsTrace.h"
#include "CacheManager.h"
#include "ICacheInterface.h"



CCacheManager  CCacheManager::g_cCacheManager;

CCacheManager::CCacheManager(void) : m_ulCacheID(0)	
{	
	WELS_INFO_TRACE("CCacheManager::CCacheManager" << ", " << this);
}

CCacheManager::~CCacheManager(void)
{
	CACHE_MAP_TYPE::iterator it = m_CacheMap.begin();
	for(; it != m_CacheMap.end();)
	{
		ISwitchCache *cache = it->second;
		if(cache == NULL)
		{
			m_CacheMap.erase(it++);
			continue;
		}
		delete cache;
		cache = NULL;
		m_CacheMap.erase(it++);
	}

	WELS_INFO_TRACE("CCacheManager::~CCacheManager" << ", " << this);
}

CCacheManager & CCacheManager::Instance()
{
	return g_cCacheManager;
}

unsigned long CCacheManager::RegisterCache(ISwitchCache *cache)
{
	unsigned long ulID = m_ulCacheID;

	m_CacheMap[ulID] = cache;

	m_ulCacheID++;

	WELS_INFO_TRACE("CCacheManager::RegisterCache CacheID=" << ulID << ", " << this);
	return ulID;
}
	
void CCacheManager::UnRegisterCache(unsigned long id)
{
	WELS_INFO_TRACE("CCacheManager::UnRegisterCache CacheID=" << id << ", " << this);

	CACHE_MAP_TYPE::iterator it = m_CacheMap.find(id);
	if(it != m_CacheMap.end())
	{
		ISwitchCache *cache = it->second;
		if(cache != NULL)
			delete cache;
		m_CacheMap.erase(it);
	}
	else
	{
		WELS_ERROR_TRACE("CCacheManager::UnRegisterCache CacheID=" << id << "cann't find, " << this);
	}
}

int CCacheManager::SendCacheData()
{
	//WSE_INFO_TRACE("CCacheHandleManager::SendCacheData");
	CACHE_MAP_TYPE::iterator it = m_CacheMap.begin();
	for(; it != m_CacheMap.end(); it++)
	{
		ISwitchCache *cache = it->second;
		if(cache != NULL)
		{
			cache->OnSendCacheData();
		}
		else
		{
			WELS_INFO_TRACE("CCacheHandleManager::SendCacheData cache is null");
		}
	}
	
	return 0;
}


