#include "simulator.h"
#include "cache.h"
#include "last_level_cache.h"
#include "cache_set_srrip.h"
#include "log.h"

// Cache class
// constructors/destructors
Cache::Cache(
   String name,
   String cfgname,
   core_id_t core_id,
   UInt32 num_sets,
   UInt32 associativity,
   UInt32 cache_block_size,
   String replacement_policy,
   cache_t cache_type,
   hash_t hash,
   FaultInjector *fault_injector,
   AddressHomeLookup *ahl)
:
   Cache(name, num_sets, associativity, cache_block_size, replacement_policy, cache_type, hash, fault_injector, ahl,
      [&] {
            const auto policy = CacheSet::parsePolicyType(replacement_policy);
            const auto num_attempts = CacheSet::getNumQBSAttempts(policy, cfgname, core_id);
            const auto num_bits = CacheSetSRRIP::getNumBits(policy, cfgname, core_id);

            m_sets = new CacheSet*[num_sets];
            m_set_info = CacheSet::createCacheSetInfo(name, cfgname, core_id, policy, associativity);

            for (UInt32 i = 0; i < num_sets; i++) {
               m_sets[i] = CacheSet::createCacheSet(policy, cache_type, associativity, cache_block_size, m_set_info, num_attempts, num_bits);
            }
      })
{ }

Cache::Cache(const String& name,
             const UInt32 num_sets,
             const UInt32 associativity,
             const UInt32 cache_block_size,
             const String& replacement_policy,
             const cache_t cache_type,
             const hash_t hash,
             FaultInjector *fault_injector,
             AddressHomeLookup *ahl,
             const std::function<void()>& createSets)
:
   CacheBase(name, num_sets, associativity, cache_block_size, hash, ahl),
   m_enabled(false),
   m_num_accesses(0),
   m_num_hits(0),
   m_cache_type(cache_type),
   m_replacement_policy(CacheSet::parsePolicyType(replacement_policy)),
   m_sets(nullptr),
   m_set_info(nullptr),
   m_fault_injector(fault_injector)
{
   createSets();

   #ifdef ENABLE_SET_USAGE_HIST
   m_set_usage_hist = new UInt64[m_num_sets];
   for (UInt32 i = 0; i < m_num_sets; i++)
      m_set_usage_hist[i] = 0;
   #endif
}

Cache::~Cache()
{
   #ifdef ENABLE_SET_USAGE_HIST
   printf("Cache %s set usage:", m_name.c_str());
   for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
      printf(" %" PRId64, m_set_usage_hist[i]);
   printf("\n");
   delete [] m_set_usage_hist;
   #endif

   if (m_set_info)
      delete m_set_info;

   for (SInt32 i = 0; i < (SInt32) m_num_sets; i++)
      delete m_sets[i];
   delete [] m_sets;
}

Lock&
Cache::getSetLock(IntPtr addr)
{
   IntPtr tag;
   UInt32 set_index;

   splitAddress(addr, tag, set_index);
   assert(set_index < m_num_sets);

   return m_sets[set_index]->getLock();
}

bool
Cache::invalidateSingleLine(IntPtr addr)
{
   IntPtr tag;
   UInt32 set_index;

   splitAddress(addr, tag, set_index);
   assert(set_index < m_num_sets);

   return m_sets[set_index]->invalidate(tag);
}

CacheBlockInfo*
Cache::accessSingleLine(IntPtr addr, access_t access_type,
      Byte* buff, UInt32 bytes, SubsecondTime now, bool update_replacement)
{
   //assert((buff == NULL) == (bytes == 0));

   IntPtr tag;
   UInt32 set_index;
   UInt32 line_index = -1;
   UInt32 block_offset;

   splitAddress(addr, tag, set_index, block_offset);

   CacheSet* set = m_sets[set_index];
   CacheBlockInfo* cache_block_info = set->find(tag, &line_index);

   if (cache_block_info == NULL)
      return NULL;

   if (access_type == LOAD)
   {
      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
      if (m_fault_injector)
         m_fault_injector->preRead(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);

      set->read_line(line_index, block_offset, buff, bytes, update_replacement);
   }
   else
   {
      set->write_line(line_index, block_offset, buff, bytes, update_replacement);

      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into buff instead
      if (m_fault_injector)
         m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, bytes, (Byte*)m_sets[set_index]->getDataPtr(line_index, block_offset), now);
   }

   return cache_block_info;
}

void
Cache::insertSingleLine(IntPtr addr, Byte* fill_buff,
      bool* eviction, IntPtr* evict_addr,
      CacheBlockInfo* evict_block_info, Byte* evict_buff,
      SubsecondTime now)
{
   insertSingleLine(addr, fill_buff, eviction, evict_addr, evict_block_info, evict_buff, now, nullptr);
}

void
Cache::insertSingleLine(IntPtr addr, Byte* fill_buff,
      bool* eviction, IntPtr* evict_addr,
      CacheBlockInfo* evict_block_info, Byte* evict_buff,
      SubsecondTime now, CacheCntlr *cntlr)
{
   IntPtr tag;
   UInt32 set_index;
   splitAddress(addr, tag, set_index);

   CacheBlockInfo* cache_block_info = CacheBlockInfo::create(m_cache_type);
   cache_block_info->setTag(tag);

   m_sets[set_index]->insert(cache_block_info, fill_buff,
         eviction, evict_block_info, evict_buff, cntlr);
   *evict_addr = tagToAddress(evict_block_info->getTag());

   if (m_fault_injector) {
      // NOTE: no callback is generated for read of evicted data
      UInt32 line_index = -1;
      __attribute__((unused)) CacheBlockInfo* res = m_sets[set_index]->find(tag, &line_index);
      LOG_ASSERT_ERROR(res != NULL, "Inserted line no longer there?");

      m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, m_sets[set_index]->getBlockSize(), (Byte*)m_sets[set_index]->getDataPtr(line_index, 0), now);
   }

   #ifdef ENABLE_SET_USAGE_HIST
   ++m_set_usage_hist[set_index];
   #endif

   delete cache_block_info;
}


// Single line cache access at addr
CacheBlockInfo*
Cache::peekSingleLine(IntPtr addr)
{
   IntPtr tag;
   UInt32 set_index;
   splitAddress(addr, tag, set_index);

   return m_sets[set_index]->find(tag);
}

void
Cache::updateCounters(bool cache_hit)
{
   if (m_enabled)
   {
      m_num_accesses ++;
      if (cache_hit)
         m_num_hits ++;
   }
}

void
Cache::updateHits(Core::mem_op_t mem_op_type, UInt64 hits)
{
   if (m_enabled)
   {
      m_num_accesses += hits;
      m_num_hits += hits;
   }
}

/**
 * Get percentage (0..1) of modified blocks in cache.
 * Added by Kleber Kruger
 */
float
Cache::getCapacityUsed() const
{
   UInt32 count = 0;
   for (UInt32 i = 0; i < m_num_sets; i++)
   {
      for (UInt32 j = 0; j < m_associativity; j++)
      {
         if (peekBlock(i, j)->isDirty())
            count++;
      }
   }
   return static_cast<float>(count) / static_cast<float>(m_num_sets * m_associativity);
}

/**
 * Get percentage (0..1) of modified blocks in cache.
 * Added by Kleber Kruger
 */
float
Cache::getSetCapacityUsed(const UInt32 index) const
{
   UInt32 count = 0;
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (m_sets[index]->peekBlock(i)->isDirty())
         count++;
   }
   return static_cast<float>(count) / static_cast<float>(m_associativity);
}

Cache*
Cache::create(const String& name,
              const String& cfgname,
              const core_id_t core_id,
              const UInt32 num_sets,
              const UInt32 associativity,
              const UInt32 cache_block_size,
              const String& replacement_policy,
              const cache_t cache_type,
              const hash_t hash,
              FaultInjector *fault_injector,
              AddressHomeLookup *ahl)
{
   if (LLCDonuts::isDonutsLLC(cfgname))
   {
      return new LLCDonuts(name, cfgname, core_id, num_sets, associativity, cache_block_size,
                                        replacement_policy, cache_type, hash, fault_injector, ahl);
   }

   return new Cache(name, cfgname, core_id, num_sets, associativity, cache_block_size,
                 replacement_policy, cache_type, hash, fault_injector, ahl);
}
