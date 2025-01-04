#include "cache_set_donuts.h"

#include "cache.h"
#include "cache_set_lru.h"
#include "config.hpp"
#include "simulator.h"

using namespace donuts;

CacheSetDonuts::CacheSetDonuts(const CacheBase::cache_t cache_type,
                               const UInt32 index,
                               const UInt32 associativity,
                               const UInt32 threshold,
                               const UInt32 block_size) :
    CacheSet(cache_type, associativity, block_size),
    m_index(index),
    m_threshold(threshold)
{
   LOG_ASSERT_ERROR(m_threshold >= 1, "The associativity set threshold must guarantee at least one cache block. "
                                      "For a cache of associativity = %u, the min-value is %.1f");
}

CacheSetDonuts::~CacheSetDonuts() = default;

bool
CacheSetDonuts::isValidReplacement(const UInt32 index)
{
   const auto state = m_cache_block_info_array[index]->getCState();
   return state != CacheState::SHARED_UPGRADING && state != CacheState::MODIFIED;
}

UInt32
CacheSetDonuts::getAmountDirtyBlocks() const
{
   UInt32 num_modified = 0;
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_cache_block_info_array[i]->isDirty())
         num_modified++;
   }
   return num_modified;
}

float
CacheSetDonuts::getThreshold(const String& cfgname, const core_id_t core_id)
{
   const auto key   = cfgname + "/cache_set_threshold";
   const auto value = Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloatArray(key, core_id)) :
                                                     DEFAULT_CACHE_SET_THRESHOLD;
   LOG_ASSERT_ERROR(value > 0.0 && value <= 1.0, "The cache set threshold must be a value greater than 0.0 and less than or equal to 1.0");
   return value;
}

CacheSet*
CacheSetDonuts::createCacheSet(const UInt32 index,
                               const CacheBase::ReplacementPolicy replacement_policy,
                               const CacheBase::cache_t cache_type,
                               const UInt32 associativity,
                               const UInt32 threshold,
                               const UInt32 block_size,
                               CacheSetInfo* set_info,
                               const UInt8 num_attempts,
                               const UInt8 num_bits_rrip)
{
   switch (replacement_policy)
   {
      case CacheBase::LRU:
         return new CacheSetLRU(cache_type, index, associativity, threshold, block_size,
                                dynamic_cast<CacheSetInfoLRU*>(set_info), num_attempts);
      case CacheBase::ROUND_ROBIN:
      case CacheBase::LRU_QBS:
      case CacheBase::NRU:
      case CacheBase::MRU:
      case CacheBase::NMRU:
      case CacheBase::PLRU:
      case CacheBase::SRRIP:
      case CacheBase::SRRIP_QBS:
      case CacheBase::RANDOM:
         LOG_PRINT_ERROR("Cache replacement policy not yet implemented: %i", replacement_policy)

      default:
         LOG_PRINT_ERROR("Unrecognized Cache Replacement Policy: %i", replacement_policy);
   }
}
