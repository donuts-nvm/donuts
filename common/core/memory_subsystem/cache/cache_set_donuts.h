#pragma once

#include "cache_set.h"

class CacheSetDonuts : public virtual CacheSet
{
public:
   CacheSetDonuts(CacheBase::cache_t cache_type,
                  UInt32 index, UInt32 associativity, UInt32 blocksize,
                  float cache_set_threshold = DEFAULT_CACHE_SET_THRESHOLD);

   ~CacheSetDonuts() override;

   static CacheSet* createCacheSet(UInt32 index,
                                   String cfgname,
                                   core_id_t core_id,
                                   String replacement_policy,
                                   CacheBase::cache_t cache_type,
                                   UInt32 associativity,
                                   UInt32 blocksize,
                                   CacheSetInfo* set_info = nullptr);

protected:
   static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;
   UInt32 m_index;
   float m_cache_set_threshold;

   bool isValidReplacement(UInt32 index) override;

   static float getCacheSetThreshold(const String& cfgname, core_id_t core_id);
};
