#ifndef CACHE_H
#define CACHE_H

#include "cache_base.h"
#include "cache_set.h"
#include "cache_block_info.h"
#include "utils.h"
#include "hash_map_set.h"
#include "cache_perf_model.h"
#include "shmem_perf_model.h"
#include "log.h"
#include "core.h"
#include "fault_injection.h"

// Define to enable the set usage histogram
//#define ENABLE_SET_USAGE_HIST

class Cache : public CacheBase
{
   protected:
      Cache(const String& name,
            UInt32 num_sets,
            UInt32 associativity,
            UInt32 cache_block_size,
            const String& replacement_policy,
            cache_t cache_type,
            hash_t hash,
            FaultInjector *fault_injector,
            AddressHomeLookup *ahl,
            const std::function<void()>& createSets);

      bool m_enabled;

      // Cache counters
      UInt64 m_num_accesses;
      UInt64 m_num_hits;

      // Generic Cache Info
      cache_t m_cache_type;
      ReplacementPolicy m_replacement_policy;
      CacheSet** m_sets;
      CacheSetInfo* m_set_info;

      FaultInjector *m_fault_injector;

      #ifdef ENABLE_SET_USAGE_HIST
      UInt64* m_set_usage_hist;
      #endif

   public:

      // constructors/destructors
      Cache(String name,
            String cfgname,
            core_id_t core_id,
            UInt32 num_sets,
            UInt32 associativity, UInt32 cache_block_size,
            String replacement_policy,
            cache_t cache_type,
            hash_t hash = CacheBase::HASH_MASK,
            FaultInjector *fault_injector = NULL,
            AddressHomeLookup *ahl = NULL);
      ~Cache();

      Lock& getSetLock(IntPtr addr);

      bool invalidateSingleLine(IntPtr addr);
      CacheBlockInfo* accessSingleLine(IntPtr addr,
            access_t access_type, Byte* buff, UInt32 bytes, SubsecondTime now, bool update_replacement);
      void insertSingleLine(IntPtr addr, Byte* fill_buff,
            bool* eviction, IntPtr* evict_addr,
            CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now);
      virtual void insertSingleLine(IntPtr addr, Byte* fill_buff,
            bool* eviction, IntPtr* evict_addr,
            CacheBlockInfo* evict_block_info, Byte* evict_buff, SubsecondTime now, CacheCntlr *cntlr);
      CacheBlockInfo* peekSingleLine(IntPtr addr);

      CacheBlockInfo* peekBlock(UInt32 set_index, UInt32 way) const { return m_sets[set_index]->peekBlock(way); }

      // Update Cache Counters
      void updateCounters(bool cache_hit);
      void updateHits(Core::mem_op_t mem_op_type, UInt64 hits);

      void enable() { m_enabled = true; }
      void disable() { m_enabled = false; }

      [[nodiscard]] ReplacementPolicy getReplacementPolicy() const { return m_replacement_policy; }
      [[nodiscard]] float getCapacityUsed() const;
      [[nodiscard]] float getSetCapacityUsed(UInt32 index) const;

      static Cache*
      create(const String& name,
             const String& cfgname,
             core_id_t core_id,
             UInt32 num_sets,
             UInt32 associativity,
             UInt32 cache_block_size,
             const String& replacement_policy,
             cache_t cache_type,
             hash_t hash = HASH_MASK,
             FaultInjector *fault_injector = nullptr,
             AddressHomeLookup *ahl = nullptr);
};

template <class T>
UInt32 moduloHashFn(T key, UInt32 hash_fn_param, UInt32 num_buckets)
{
   return (key >> hash_fn_param) % num_buckets;
}

#endif /* CACHE_H */
