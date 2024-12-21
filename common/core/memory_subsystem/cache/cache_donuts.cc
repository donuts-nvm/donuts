#include "cache_donuts.h"
#include "cache_set_donuts.h"
#include "config.hpp"
#include "simulator.h"

CacheDonuts::CacheDonuts(const String& name,
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
    Cache(name, cfgname, core_id, num_sets, associativity, cache_block_size,
          replacement_policy, cache_type, hash, fault_injector, ahl),
    m_set_threshold(CacheSetDonuts::getCacheSetThreshold(cfgname, core_id)),
    m_threshold(getCacheThreshold(cfgname))
{
   LOG_ASSERT_ERROR(cache_type != CacheBase::SHARED_CACHE, "Invalid cache type")
   LOG_ASSERT_ERROR(m_threshold <= m_set_threshold, "The cache_threshold must be less than or equal to cache_set_threshold")
}

void
CacheDonuts::insertSingleLine(IntPtr addr, Byte* fill_buff,
                              bool* eviction, IntPtr* evict_addr,
                              CacheBlockInfo* evict_block_info, Byte* evict_buff,
                              SubsecondTime now)
{
   insertSingleLine(addr, fill_buff, eviction, evict_addr, evict_block_info, evict_buff, now, nullptr);
}

void
CacheDonuts::insertSingleLine(IntPtr addr, Byte* fill_buff,
                              bool* eviction, IntPtr* evict_addr,
                              CacheBlockInfo* evict_block_info, Byte* evict_buff,
                              SubsecondTime now, CacheCntlr* cntlr)
{
   Cache::insertSingleLine(addr, fill_buff, eviction, evict_addr, evict_block_info, evict_buff, now, cntlr);
}

float
CacheDonuts::getCacheThreshold(const String& cfgname)
{
   const String key = cfgname + "/cache_threshold";
   return Sim()->getCfg()->hasKey(key) ? static_cast<float>(Sim()->getCfg()->getFloat(key)) : DEFAULT_THRESHOLD;
}

bool
CacheDonuts::isDonutsLLC(const String& cfgname)
{
   if (Sim()->getProjectType() == ProjectType::DONUTS)
   {
      const UInt32 levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
      const String last   = levels == 1 ? "perf_model/l1_dcache" : "perf_model/l" + String(std::to_string(levels).c_str()) + "_cache";
      return cfgname == last;
   }
   return false;
}
