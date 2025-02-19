#pragma once

#include "cache.h"

namespace donuts
{

enum class PersistencePolicy
{
   SEQUENTIAL,
   FULLEST_FIRST,
   BALANCED
};

class LastLevelCache final : public Cache
{
public:

   LastLevelCache(const String& name,
                  const String& cfgname,
                  core_id_t core_id,
                  UInt32 num_sets,
                  UInt32 associativity,
                  UInt32 cache_block_size,
                  const String& replacement_policy,
                  cache_t cache_type,
                  hash_t hash                   = HASH_MASK,
                  FaultInjector* fault_injector = nullptr,
                  AddressHomeLookup* ahl        = nullptr);

   void insertSingleLine(IntPtr addr, Byte* fill_buff, bool* eviction, IntPtr* evict_addr,
                         CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now, ::CacheCntlr* cntlr) override;

   [[nodiscard]] UInt32 getLastInsertedIndex() const { return m_last_inserted_index; }

   [[nodiscard]] float getThreshold() const { return m_threshold; }
   [[nodiscard]] float getSetThreshold() const { return m_set_threshold; }
   [[nodiscard]] PersistencePolicy getPersistencePolicy() const { return m_persistence_policy; }

   [[nodiscard]] static bool isDonutsLLC(const String& cfgname);

private:
   static constexpr auto DEFAULT_CACHE_THRESHOLD = 0.75f;
   static constexpr auto DEFAULT_PERSISTENCE_POLICY = "sequential";

   float m_set_threshold;
   float m_threshold;
   PersistencePolicy m_persistence_policy;
   UInt32 m_last_inserted_index;

   [[nodiscard]] static float loadThreshold(const String& cfgname, core_id_t core_id);
   [[nodiscard]] static PersistencePolicy loadPersistencePolicy(const String& cfgname, core_id_t core_id);
};

}

using LLCDonuts = donuts::LastLevelCache;
