#pragma once
#include <map>



class ISwitchCache;

class CCacheManager 
{	
public:
	CCacheManager(void);
	virtual ~CCacheManager(void);
	
	static CCacheManager & Instance();

	unsigned long RegisterCache(ISwitchCache *cache);
	void UnRegisterCache(unsigned long id);

	int SendCacheData();
	unsigned long GetCacheID() { return m_ulCacheID; }
private:
	
	typedef std::map<unsigned long, ISwitchCache*> CACHE_MAP_TYPE;
	CACHE_MAP_TYPE    m_CacheMap;

	unsigned long m_ulCacheID;

	static CCacheManager  g_cCacheManager;
};
