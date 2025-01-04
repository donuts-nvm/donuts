#pragma once

#include "cache_set.h"

namespace donuts
{

class CacheSetDonuts : public virtual CacheSet
{
public:
   CacheSetDonuts(CacheBase::cache_t cache_type,
                  UInt32 index,
                  UInt32 associativity,
                  UInt32 threshold,
                  UInt32 block_size);

   ~CacheSetDonuts() override;

   static CacheSet* createCacheSet(UInt32 index,
                                   CacheBase::ReplacementPolicy replacement_policy,
                                   CacheBase::cache_t cache_type,
                                   UInt32 associativity,
                                   UInt32 threshold,
                                   UInt32 block_size,
                                   CacheSetInfo* set_info = nullptr,
                                   UInt8 num_attempts     = 1,
                                   UInt8 num_bits_rrip    = 0);

   [[nodiscard]] UInt32 getIndex() const { return m_index; }
   [[nodiscard]] UInt32 getThreshold() const { return m_threshold; }

   static float getThreshold(const String& cfgname, core_id_t core_id);

protected:
   static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;
   const UInt32 m_index;
   const UInt32 m_threshold;

   bool isValidReplacement(UInt32 index) override;
   [[nodiscard]] UInt32 getAmountDirtyBlocks() const;
};

}
