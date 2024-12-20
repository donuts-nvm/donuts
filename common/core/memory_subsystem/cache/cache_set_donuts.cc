#include "cache_set_donuts.h"

#include "cache.h"
#include "cache_set_lrur.h"
#include "config.hpp"
#include "simulator.h"

CacheSetDonuts::CacheSetDonuts(const CacheBase::cache_t cache_type, const UInt32 index, const UInt32 associativity,
                               const UInt32 blocksize, const float cache_set_threshold) :
    CacheSet(cache_type, associativity, blocksize),
    m_index(index),
    m_cache_set_threshold(cache_set_threshold) {}

CacheSetDonuts::~CacheSetDonuts() = default;

bool
CacheSetDonuts::isValidReplacement(const UInt32 index)
{
   const auto state = m_cache_block_info_array[index]->getCState();
   return state != CacheState::SHARED_UPGRADING && state != CacheState::MODIFIED;
}

float
CacheSetDonuts::getCacheSetThreshold(const String& cfgname, const core_id_t core_id)
{
   const String key = cfgname + "/cache_set_threshold";
   return Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloatArray(key, core_id)) : DEFAULT_CACHE_SET_THRESHOLD;
}

CacheSet*
CacheSetDonuts::createCacheSet(const UInt32 index,
                               String cfgname,
                               core_id_t core_id,
                               String replacement_policy,
                               CacheBase::cache_t cache_type,
                               UInt32 associativity,
                               UInt32 blocksize,
                               CacheSetInfo* set_info)
{
   CacheBase::ReplacementPolicy policy = parsePolicyType(replacement_policy);
   switch (policy)
   {
      case CacheBase::LRU:
      case CacheBase::LRU_QBS:
         return new CacheSetLRUR(cache_type, index, associativity, blocksize, dynamic_cast<CacheSetInfoLRU*>(set_info), getNumQBSAttempts(policy, cfgname, core_id));
      default:
         LOG_PRINT_ERROR("Unrecognized Cache Replacement Policy: %i", policy);
   }
}
