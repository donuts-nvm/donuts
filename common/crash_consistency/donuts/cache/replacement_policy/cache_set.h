#pragma once

#include "../common/core/memory_subsystem/cache/cache_set.h"
#include "checkpoint_reason.h"

namespace donuts
{

class CacheSet : public virtual ::CacheSet
{
public:
   CacheSet(CacheBase::cache_t cache_type,
            UInt32 index,
            UInt32 associativity,
            UInt32 threshold,
            UInt32 block_size);

   ~CacheSet() override;

   [[nodiscard]] UInt32 getIndex() const { return m_index; }
   [[nodiscard]] UInt32 getThreshold() const { return m_threshold; }
   [[nodiscard]] float getThresholdPerc() const { return m_threshold / m_associativity; }
   void setThreshold(const UInt32 threshold)
   {
      LOG_ASSERT_ERROR(threshold >= 1 && threshold <= m_associativity, "Invalid cache set threshold: %u/%u", threshold, m_associativity);
      m_threshold = threshold;
   }

   static float loadThreshold(const String& cfgname, core_id_t core_id);

   static CacheSet* createCacheSet(UInt32 index,
                                   CacheBase::ReplacementPolicy replacement_policy,
                                   CacheBase::cache_t cache_type,
                                   UInt32 associativity,
                                   double threshold,
                                   UInt32 block_size,
                                   CacheSetInfo* set_info,
                                   UInt8 num_attempts,
                                   UInt8 num_bits_rrip);

   static CacheSet* createCacheSet(UInt32 index,
                                   CacheBase::ReplacementPolicy replacement_policy,
                                   CacheBase::cache_t cache_type,
                                   UInt32 associativity,
                                   UInt32 threshold,
                                   UInt32 block_size,
                                   CacheSetInfo* set_info,
                                   UInt8 num_attempts,
                                   UInt8 num_bits_rrip);

protected:
   static constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;
   const UInt32 m_index;
   UInt32 m_threshold;

   bool isValidReplacement(UInt32 index) override;
   [[nodiscard]] UInt32 countDirtyBlocks() const;
};

}

using CacheSetDonuts = donuts::CacheSet;
