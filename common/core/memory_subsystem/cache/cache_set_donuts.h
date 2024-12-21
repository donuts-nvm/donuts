#pragma once

#include "cache_set.h"

class CacheSetDonuts : public virtual CacheSet
{
public:
   CacheSetDonuts(CacheBase::cache_t cache_type,
                  UInt32 index, UInt32 associativity, UInt32 blocksize,
                  float set_threshold);

   ~CacheSetDonuts() override;

   static CacheSet* createCacheSet(UInt32 index,
                                   CacheBase::ReplacementPolicy replacement_policy,
                                   CacheBase::cache_t cache_type,
                                   UInt32 associativity,
                                   UInt32 blocksize,
                                   float set_threshold,
                                   CacheSetInfo* set_info = nullptr,
                                   UInt8 num_attempts = 1);

   [[nodiscard]] UInt32 getIndex() const { return m_index; }
   [[nodiscard]] float getCacheSetThreshold() const { return m_cache_set_threshold; }
   [[nodiscard]] float getCacheSetUsed() const;

   static float getCacheSetThreshold(const String& cfgname, core_id_t core_id);

protected:
   static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;
   UInt32 m_index;
   float m_cache_set_threshold;

   bool isValidReplacement(UInt32 index) override;
};
