#pragma once
#include <cache.h>

class CacheDonuts : public Cache
{
public:
   CacheDonuts(const String& name,
               const String& cfgname,
               core_id_t core_id,
               UInt32 num_sets,
               UInt32 associativity, UInt32 cache_block_size,
               const String& replacement_policy,
               cache_t cache_type,
               hash_t hash                   = HASH_MASK,
               FaultInjector* fault_injector = nullptr,
               AddressHomeLookup* ahl        = nullptr);

   void insertSingleLine(IntPtr addr, Byte* fill_buff, bool* eviction, IntPtr* evict_addr,
                         CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now) override;

   void insertSingleLine(IntPtr addr, Byte* fill_buff, bool* eviction, IntPtr* evict_addr,
                         CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now, CacheCntlr* cntlr) override;

   [[nodiscard]] float getThreshold() const { return m_threshold; }
   [[nodiscard]] float getSetThreshold() const { return m_set_threshold; }

   [[nodiscard]] static bool isDonutsLLC(const String& cfgname);
   [[nodiscard]] static float getCacheThreshold(const String& cfgname);

private:
   static constexpr float DEFAULT_THRESHOLD     = 0.75;
   static constexpr float DEFAULT_SET_THRESHOLD = 1.0;

   float m_set_threshold;
   float m_threshold;
};
