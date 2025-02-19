#include "last_level_cache.h"
#include "../donuts/cache/replacement_policy/cache_set.h"
#include "cache_set_srrip.h"
#include "config.hpp"
#include "epoch_info.h"
#include "simulator.h"

#include <format>

namespace donuts
{

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
    Cache(name, num_sets, associativity, cache_block_size, CacheSet::parsePolicyType(replacement_policy),
          cache_type, hash, fault_injector, ahl,
          [&](auto sets, auto set_info) {
             const auto policy       = CacheSet::parsePolicyType(replacement_policy);
             const auto num_attempts = CacheSet::getNumQBSAttempts(policy, cfgname, core_id);
             const auto num_bits     = CacheSetSRRIP::getNumBits(policy, cfgname, core_id);
             const auto threshold    = CacheSet::loadThreshold(cfgname, core_id);

             set_info = CacheSet::createCacheSetInfo(name, cfgname, core_id, policy, associativity);

             for (UInt32 index = 0; index < num_sets; index++)
             {
                sets[index] = CacheSet::createCacheSet(index, policy, cache_type, associativity, threshold,
                                                       cache_block_size, set_info, num_attempts, num_bits);
             }
          }),
    m_set_threshold(CacheSetDonuts::loadThreshold(cfgname, core_id)),
    m_threshold(loadThreshold(cfgname, core_id)),
    m_persistence_policy(loadPersistencePolicy(cfgname, core_id)),
    m_last_inserted_index(0)
{
   LOG_ASSERT_ERROR(cache_type == CacheBase::SHARED_CACHE, "Invalid cache type")
   LOG_ASSERT_WARNING(m_threshold <= m_set_threshold,
                      "The cache threshold is unreachable because its value is greater than the cache set threshold")
}

void LastLevelCache::insertSingleLine(const IntPtr addr, Byte* fill_buff, bool* eviction, IntPtr* evict_addr,
                                      CacheBlockInfo* evict_block_info, Byte* evict_buff, const SubsecondTime now,
                                      ::CacheCntlr* cntlr)
{
   Cache::insertSingleLine(addr, fill_buff, eviction, evict_addr, evict_block_info, evict_buff, now, cntlr);

   IntPtr tag;
   splitAddress(addr, tag, m_last_inserted_index);
   if (getCapacityUsed() >= m_threshold)
   {
      cntlr->checkpoint(CheckpointReason::CACHE_THRESHOLD, m_last_inserted_index);
   }
}

float LastLevelCache::loadThreshold(const String& cfgname, const core_id_t core_id)
{
   const auto key   = cfgname + "/cache_threshold";
   const auto value = Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloatArray(key, core_id)) :
                                                     DEFAULT_CACHE_THRESHOLD;
   LOG_ASSERT_ERROR(value > 0.0 && value <= 1.0, "The cache threshold must be a value greater than 0.0 and less than or equal to 1.0");
   return value;
}

PersistencePolicy LastLevelCache::loadPersistencePolicy(const String& cfgname, const core_id_t core_id)
{
   const auto key   = cfgname + "/persistence_policy";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getStringArray(key, core_id) :
                                                     DEFAULT_PERSISTENCE_POLICY;

   if (value == "sequential") return PersistencePolicy::SEQUENTIAL;
   if (value == "fullest_first") return PersistencePolicy::FULLEST_FIRST;
   if (value == "balanced") return PersistencePolicy::BALANCED;

   LOG_ASSERT_ERROR(false, "Persistence policy not found: %", value.c_str());
}

bool LastLevelCache::isDonutsLLC(const String& cfgname)
{
   if (Sim()->getProjectType() == ProjectType::DONUTS)
   {
      const UInt32 lvl = Sim()->getCfg()->getInt("perf_model/cache/levels");
      const String llc = "perf_model/" + String(lvl == 1 ? "l1_dcache" : std::format("l{}_cache", lvl).c_str());
      return cfgname == llc;
   }
   return false;
}

}// namespace donuts
