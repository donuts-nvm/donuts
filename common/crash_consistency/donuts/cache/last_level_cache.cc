#include "last_level_cache.h"
#include "cache_set_donuts.h"
#include "cache_set_srrip.h"
#include "config.hpp"
#include "simulator.h"

#include <boost/math/policies/policy.hpp>

using namespace donuts;

LastLevelCache::LastLevelCache(const String& name,
                               const String& cfgname,
                               const core_id_t core_id,
                               const UInt32 num_sets,
                               const UInt32 associativity,
                               const UInt32 cache_block_size,
                               const String& replacement_policy,
                               const cache_t cache_type,
                               const hash_t hash,
                               FaultInjector* fault_injector,
                               AddressHomeLookup* ahl) :
    Cache(name, num_sets, associativity, cache_block_size, replacement_policy, cache_type, hash, fault_injector, ahl,
          [&]
          {
             const auto policy = CacheSet::parsePolicyType(replacement_policy);
             m_sets            = new CacheSet*[num_sets];
             m_set_info        = CacheSet::createCacheSetInfo(name, cfgname, core_id, policy, associativity);

             const auto num_attempts  = CacheSet::getNumQBSAttempts(policy, cfgname, core_id);
             const auto num_bits      = CacheSetSRRIP::getNumBits(policy, cfgname, core_id);
             const auto set_threshold = CacheSetDonuts::getThreshold(cfgname, core_id);

             const auto threshold = static_cast<UInt32>(set_threshold * static_cast<float>(associativity));
             for (UInt32 index = 0; index < num_sets; index++)
             {
                m_sets[index] = CacheSetDonuts::createCacheSet(index, policy, cache_type, associativity, threshold,
                                                               cache_block_size, m_set_info, num_attempts, num_bits);
             }
          }),
    m_set_threshold(CacheSetDonuts::getThreshold(cfgname, core_id)),
    m_threshold(getThreshold(cfgname))
{
   LOG_ASSERT_ERROR(cache_type == CacheBase::SHARED_CACHE, "Invalid cache type")
   LOG_ASSERT_WARNING(m_threshold <= m_set_threshold,
                      "The cache threshold is unreachable because its value is greater than the cache set threshold")
}

void
LastLevelCache::insertSingleLine(const IntPtr addr, Byte* fill_buff, bool* eviction, IntPtr* evict_addr,
                                 CacheBlockInfo* evict_block_info, Byte* evict_buff, const SubsecondTime now,
                                 CacheCntlr* cntlr)
{
   Cache::insertSingleLine(addr, fill_buff, eviction, evict_addr, evict_block_info, evict_buff, now, cntlr);

   if (getCapacityUsed() >= m_threshold)
   {
      IntPtr tag;
      UInt32 set_index;
      splitAddress(addr, tag, set_index);
      cntlr->checkpoint(CheckpointReason::CACHE_THRESHOLD, set_index);
   }
}

float
LastLevelCache::getThreshold(const String& cfgname)
{
   const auto key   = cfgname + "/cache_threshold";
   const auto value = Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloat(key)) :
                                                     DEFAULT_CACHE_THRESHOLD;
   LOG_ASSERT_ERROR(value > 0.0 && value <= 1.0, "The cache threshold must be a value greater than 0.0 and less than or equal to 1.0");
   return value;
}

bool
LastLevelCache::isDonutsLLC(const String& cfgname)
{
   if (Sim()->getProjectType() == ProjectType::DONUTS)
   {
      const UInt32 levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
      const String last   = levels == 1 ? "perf_model/l1_dcache" : "perf_model/l" + String(std::to_string(levels).c_str()) + "_cache";
      return cfgname == last;
   }
   return false;
}
