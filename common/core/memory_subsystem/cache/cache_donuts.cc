// #include "cache_donuts.h"
//
// #include "config.hpp"
// #include "simulator.h"
//
// CacheDonuts::CacheDonuts(String name,
//                          String cfgname,
//                          core_id_t core_id,
//                          UInt32 num_sets,
//                          UInt32 associativity,
//                          UInt32 cache_block_size,
//                          String replacement_policy,
//                          cache_t cache_type,
//                          float set_threshold,
//                          float cache_threshold,
//                          hash_t hash,
//                          FaultInjector* fault_injector,
//                          AddressHomeLookup* ahl) :
//     Cache(name, cfgname, core_id, num_sets, associativity, cache_block_size, replacement_policy,
//           SHARED_CACHE, hash, fault_injector, ahl),
//     m_cache_set_threshold(set_threshold),
//     m_cache_threshold(cache_threshold) {}
//
// void
// Cache::insertSingleLine(IntPtr addr, Byte* fill_buff,
//                         bool* eviction, IntPtr* evict_addr,
//                         CacheBlockInfo* evict_block_info, Byte* evict_buff,
//                         SubsecondTime now, CacheCntlr* cntlr)
// {
//    IntPtr tag;
//    UInt32 set_index;
//    splitAddress(addr, tag, set_index);
//
//    CacheBlockInfo* cache_block_info = CacheBlockInfo::create(m_cache_type);
//    cache_block_info->setTag(tag);
//
//    m_sets[set_index]->insert(cache_block_info, fill_buff,
//                              eviction, evict_block_info, evict_buff, cntlr);
//    *evict_addr = tagToAddress(evict_block_info->getTag());
//
//    if (m_fault_injector)
//    {
//       // NOTE: no callback is generated for read of evicted data
//       UInt32 line_index                           = -1;
//       __attribute__((unused)) CacheBlockInfo* res = m_sets[set_index]->find(tag, &line_index);
//       LOG_ASSERT_ERROR(res != nullptr, "Inserted line no longer there?");
//
//       m_fault_injector->postWrite(addr, set_index * m_associativity + line_index, m_sets[set_index]->getBlockSize(), (Byte*) m_sets[set_index]->getDataPtr(line_index, 0), now);
//    }
//
// #ifdef ENABLE_SET_USAGE_HIST
//    ++m_set_usage_hist[set_index];
// #endif
//
//    delete cache_block_info;
// }
//
// /**
//  * Get percentage (0..1) of modified blocks in cache.
//  * Added by Kleber Kruger
//  */
// float
// Cache::getCapacityUsed() const
// {
//    UInt32 count = 0;
//    for (UInt32 i = 0; i < m_num_sets; i++)
//    {
//       for (UInt32 j = 0; j < m_associativity; j++)
//       {
//          if (peekBlock(i, j)->isDirty())
//             count++;
//       }
//    }
//    return static_cast<float>(count) / static_cast<float>(m_num_sets * m_associativity);
// }
//
// /**
//  * Get percentage (0..1) of modified blocks in cache.
//  * Added by Kleber Kruger
//  */
// float
// Cache::getSetCapacityUsed(const UInt32 index) const
// {
//    UInt32 count = 0;
//    for (UInt32 i = 0; i < m_associativity; i++)
//    {
//       if (m_sets[index]->peekBlock(i)->isDirty())
//          count++;
//    }
//    return static_cast<float>(count) / static_cast<float>(m_associativity);
// }
//
// bool
// Cache::isDonutsAndLLC(const String& cfgname)
// {
//    const UInt32 levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
//    const String last   = levels == 1 ? "perf_model/l1_dcache" : "perf_model/l" + String(std::to_string(levels).c_str()) + "_cache";
//    return cfgname == last;
// }
