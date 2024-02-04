//#include "cache_cntlr_donuts_old.h"
//#include "hooks_manager.h"
//#include "memory_manager.h"
//#include "nvm_cntlr_donuts.h"
//
//namespace ParametricDramDirectoryMSI
//{
//
//CacheCntlrDonuts::CacheCntlrDonuts(MemComponent::component_t mem_component,
//                                   String name,
//                                   core_id_t core_id,
//                                   MemoryManager* memory_manager,
//                                   AddressHomeLookup* tag_directory_home_lookup,
//                                   Semaphore* user_thread_sem,
//                                   Semaphore* network_thread_sem,
//                                   UInt32 cache_block_size,
//                                   CacheParameters& cache_params,
//                                   ShmemPerfModel* shmem_perf_model,
//                                   bool is_last_level_cache,
//                                   EpochCntlr* epoch_cntlr) :
//    CacheCntlrWrBuff(mem_component, name, core_id, memory_manager, tag_directory_home_lookup,
//                     user_thread_sem, network_thread_sem, cache_block_size, cache_params,
//                     shmem_perf_model, is_last_level_cache),
//    m_epoch_cntlr(epoch_cntlr),
//    m_persistence_policy(getPersistencePolicy())
//{
////   printf("Cache %s (%p) | CacheCntlr (%p) | Core %u/%u\n", getCache()->getName().c_str(), getCache(), this, m_core_id, m_core_id_master);
//
//   if (is_last_level_cache)
//   {
//      // TODO: Implement for non-unified LLC... Use a Master Controller??
//      LOG_ASSERT_ERROR(m_core_id_master == 0, "DONUTS does not allow non-unified LLC yet");
//
//      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT, _checkpoint_timeout, (UInt64) this);
//      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS, _checkpoint_instr, (UInt64) this);
//   }
//}
//
//CacheCntlrDonuts::~CacheCntlrDonuts() = default;
//
//void
//CacheCntlrDonuts::addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const
//{
////   printf("Scanning set %u: %.1f%%\n", set_index, m_master->m_cache->getSetCapacityUsed(set_index) * 100);
//   for (UInt32 way = 0; way < m_master->m_cache->getAssociativity(); way++)
//   {
//      CacheBlockInfo* block_info = m_master->m_cache->peekBlock(set_index, way);
//      if (block_info->isDirty())
//         dirty_blocks.push(block_info);
//   }
//}
//
///**
// * Select the dirty blocks from the cache according to persistence policy.
// * TODO: Instead of sending everything at once, dispatch blocks in burst according to the write buffer size.
// *
// * @param evicted_set_index
// * @return a sorted queue containing all the dirty blocks
// */
//std::queue<CacheBlockInfo*>
//CacheCntlrDonuts::selectDirtyBlocks(UInt32 evicted_set_index) const
//{
//   LOG_ASSERT_ERROR(m_persistence_policy != PersistencePolicy::BALANCED, "Persistence policy not yet implemented");
//
//   std::queue<CacheBlockInfo*> dirty_blocks;
//   std::vector<std::pair<UInt32, double> > other_sets;
//
//   addAllDirtyBlocks(dirty_blocks, evicted_set_index);
//
//   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
//   {
//      if (i == evicted_set_index) continue;
//      auto used = m_persistence_policy == FULLEST_FIRST ? m_master->m_cache->getSetCapacityUsed(i) : 1;
//      if (used > 0) other_sets.emplace_back(i, used);
//   }
//
//   if (m_persistence_policy == FULLEST_FIRST)
//      std::sort(other_sets.begin(), other_sets.end(), [](const auto& a, const auto& b) { return (a.second > b.second); });
//
//   for (auto set: other_sets)
//      addAllDirtyBlocks(dirty_blocks, set.first);
//
//   return dirty_blocks;
//}
//
//void
//CacheCntlrDonuts::checkpoint(CheckpointEvent::type_t event_type, UInt32 evicted_set_index)
//{
////   for (UInt32 i = 0; i < Sim()->getConfig()->getTotalCores(); i++)
////   {
////      auto cache_cntlr = getMemoryManager()->getCacheCntlrAt(i, m_mem_component);
////      if (cache_cntlr->isMasterCache())
////      {
////         // TODO: invocar o checkpoint para todos os controladores indicando a cache e o conjunto associativo que estourou o limiar
////         printf("Running checkpoint on CacheCntlrDonuts %p\n", cache_cntlr);
////      }
////   }
//
//   // for all last level caches... (ALL IN THE SAME TIME!)
//   auto dirty_blocks = selectDirtyBlocks(evicted_set_index);
//   if (!dirty_blocks.empty())
//   {
//      // for all last level caches...
////      printCache();
//
//      IntPtr pc = Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache() >> 4;
//      processCheckpointStart(pc);
//
//      while (!dirty_blocks.empty())
//      {
////         sendDataToDram(m_master->m_cache->tagToAddress(dirty_blocks.front()->getTag()));
//         auto cache_block = dirty_blocks.front();
//         IntPtr address = m_master->m_cache->tagToAddress(cache_block->getTag());
//         processCommit(address);
//         dirty_blocks.pop();
//      }
//
//      processCheckpointFinished(pc);
//
//      // for all last level caches...
////      printCache();
//      m_epoch_cntlr->commit();
//   }
//}
//
//void
//CacheCntlrDonuts::sendMsgTo(IntPtr address, PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t receiver_mem_component)
//{
//   core_id_t receiver = receiver_mem_component == MemComponent::TAG_DIR ? MemComponent::TAG_DIR : m_core_id_master;
//   getMemoryManager()->sendMsg(msg_type, MemComponent::LAST_LEVEL_CACHE, receiver_mem_component,
//                               m_core_id_master, receiver, /* requester and receiver */
//                               address, NULL, 0,
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//}
//
//void
//CacheCntlrDonuts::processCheckpointStart(IntPtr address)
//{
////   printf("CHECKPOINT get PC = %lX\n", address);
//   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::CHECKPOINT_START,
//                               MemComponent::LAST_LEVEL_CACHE, MemComponent::NVM,
//                               m_core_id_master, getHome(address), /* requester and receiver */
//                               address, NULL, 0,
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//}
//
//void
//CacheCntlrDonuts::processCommit(IntPtr address)
//{
//   Byte data_buf[getCacheBlockSize()];
//   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT,
//                               MemComponent::LAST_LEVEL_CACHE, MemComponent::TAG_DIR,
//                               m_core_id_master, getHome(address), /* requester and receiver */
//                               address, NULL, 0,
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//   updateCacheBlock(address, CacheState::SHARED, Transition::COHERENCY, data_buf, ShmemPerfModel::_USER_THREAD);
//}
//
//void
//CacheCntlrDonuts::processCheckpointFinished(IntPtr address)
//{
//   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::CHECKPOINT_FINISHED,
//                               MemComponent::LAST_LEVEL_CACHE, MemComponent::NVM,
//                               m_core_id_master, getHome(address), /* requester and receiver */
//                               address, NULL, 0,
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//}
//
//void
//CacheCntlrDonuts::sendByWriteBuffer(const WriteBufferEntry& entry)
//{
//   // FIXME: e se for usar write-buffer nos níveis intermediários da cache??
//   printf("Sending via write-buffer [ %lx ]...\n", entry.getAddress());
//
//   Byte data_buf[getCacheBlockSize()];
//   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::PERSIST,
//                               MemComponent::LAST_LEVEL_CACHE, MemComponent::TAG_DIR,
//                               m_core_id_master, getHome(entry.getAddress()), /* requester and receiver */
//                               entry.getAddress(), data_buf, getCacheBlockSize(),
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//}
//
////SubsecondTime
////CacheCntlrDonuts::sendDataToDram(IntPtr address)
////{
////   //   printf("Sending %lX (%c)...\n", address, getCacheBlockInfo(address)->getCStateString());
////   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT,
////                               MemComponent::LAST_LEVEL_CACHE, MemComponent::TAG_DIR,
////                               m_core_id_master, getHome(address), /* requester and receiver */
////                               address, NULL, 0,
////                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
////
////   Byte data_buf[getCacheBlockSize()];
////   updateCacheBlock(address, CacheState::SHARED, Transition::COHERENCY, data_buf, ShmemPerfModel::_USER_THREAD);
////
////   getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::PERSIST,
////                               MemComponent::LAST_LEVEL_CACHE, MemComponent::TAG_DIR,
////                               m_core_id_master, getHome(address), /* requester and receiver */
////                               address, data_buf, getCacheBlockSize(),
////                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
////   return SubsecondTime::Zero();
////}
//
//CacheCntlrDonuts::PersistencePolicy
//CacheCntlrDonuts::getPersistencePolicy()
//{
//   String param = "donuts/persistence_policy";
//   String value = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "sequential";
//
//   if (value == "sequential") return CacheCntlrDonuts::SEQUENTIAL;
//   if (value == "fullest_first") return CacheCntlrDonuts::FULLEST_FIRST;
//   if (value == "balanced") return CacheCntlrDonuts::BALANCED;
//
//   LOG_ASSERT_ERROR(false, "Persistence policy not found: %", value.c_str());
//}
//
///********************************************************************************
// * ONLY FOR DEBUG!
// ********************************************************************************/
//void CacheCntlrDonuts::printCache()
//{
//   printf("============================================================\n");
//   printf("Cache %s (%.2f%%) (%p)\n", getCache()->getName().c_str(), getCache()->getCapacityUsed() * 100, getCache());
//   printf("------------------------------------------------------------\n");
//   for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
//   {
//      printf("%s%4d  ", j == 0 ? "    " : "", j);
//   }
//   printf("\n------------------------------------------------------------\n");
//   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
//   {
//      printf("%4d ", i);
//      for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
//      {
//         auto cache_block = m_master->m_cache->peekBlock(i, j);
//         auto state       = cache_block->getCState() != CacheState::INVALID ? cache_block->getCStateString() : ' ';
//         printf("[%c %lu] ", state, cache_block->getEpochID());
//      }
//      printf("= %.1f\n", m_master->m_cache->getSetCapacityUsed(i) * 100);
//   }
//   printf("============================================================\n");
//}
//
//}