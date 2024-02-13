//#include "cache_cntlr_donuts.h"
//#include "memory_manager.h"
//#include "hooks_manager.h"
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
//
//      Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC, _interrupt, (UInt64) this);
//   }
//}
//
//CacheCntlrDonuts::~CacheCntlrDonuts() = default;
//
//void
//CacheCntlrDonuts::addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const
//{
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
//   if (!m_dirty_blocks.empty())
//   {
//      printf("AINDA TINHA PORRA NESSA BUCETA!!!\n");
//      SubsecondTime t_send = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
//      printf("CHECKPOINT STARTING in %lu\n", t_send.getNS());
//      flushAll();
//      m_epoch_cntlr->commit();
//   }
//   else
//   {
//      auto dirty_blocks = selectDirtyBlocks(evicted_set_index);
//      SubsecondTime t_send = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
//      printf("CHECKPOINT STARTING in %lu\n", t_send.getNS());
//      while (!dirty_blocks.empty())
//      {
//         auto cache_block = dirty_blocks.front();
//         t_send += SubsecondTime::NS(200);
//         m_dirty_blocks.emplace(cache_block, t_send);
//         dirty_blocks.pop();
//
//         printf("[ %lx ] (%lu)\n", m_master->m_cache->tagToAddress(cache_block->getTag()), t_send.getNS());
//      }
//      m_epoch_cntlr->commit();
//      produceWriteBuffer();
//   }
//
//
////   auto dirty_blocks = selectDirtyBlocks(evicted_set_index);
////   if (!dirty_blocks.empty())
////   {
//////      printCache();
////      while (!dirty_blocks.empty())
////      {
////         auto latency = sendDataToDram(m_master->m_cache->tagToAddress(dirty_blocks.front()->getTag()));
////         getMemoryManager()->incrElapsedTime(latency, ShmemPerfModel::_USER_THREAD);
////         dirty_blocks.pop();
////      }
////      printf("Checkpoint FINISHED %lu\n", getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD).getNS());
//////      getMemoryManager()->incrElapsedTime()
////      m_epoch_cntlr->commit();
//////      printCache();
////   }
//}
//
//void
//CacheCntlrDonuts::processCommit(IntPtr address, Byte* data_buf)
//{
//   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT, MemComponent::TAG_DIR, address, nullptr);
//   updateCacheBlock(address, CacheState::SHARED, Transition::COHERENCY, data_buf, ShmemPerfModel::_USER_THREAD);
//}
//void
//CacheCntlrDonuts::processPersist(IntPtr address, Byte* data_buf)
//{
//   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::PERSIST, MemComponent::TAG_DIR, address, data_buf);
//}
//
//void
//CacheCntlrDonuts::sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t receiver_mem_component,
//                            IntPtr address, Byte* data_buf)
//{
//   getMemoryManager()->sendMsg(msg_type,MemComponent::LAST_LEVEL_CACHE, receiver_mem_component,
//                               m_core_id_master, getHome(address), /* requester and receiver */
//                               address, data_buf, data_buf != nullptr ? getCacheBlockSize() : 0,
//                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
//}
//
//SubsecondTime // FIXME: Don't use me!
//CacheCntlrDonuts::sendDataToDram(IntPtr address)
//{
////   if (!isLastLevel())
////      return CacheCntlrWrBuff::sendDataToDram(address);
////
////   Byte data_buf[getCacheBlockSize()];
////   auto cache_block = getCacheBlockInfo(address);
////   printf("Commiting [ %lx ] (%c) (time: %lu)...\n", address, getCacheBlockInfo(address)->getCStateString(), getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD).getNS());
////   auto latency = m_writebuffer_cntlr->insert(address, 0, nullptr, getCacheBlockSize(), ShmemPerfModel::_USER_THREAD, cache_block->getEpochID());
//////   if (latency > SubsecondTime::Zero()) {
////      m_writebuffer_cntlr->print();
//////   }
////   processCommit(address, data_buf);
////   printf("Commit Latency: %lu\n", latency.getNS());
////
////   return latency;
//
//   return SubsecondTime::Zero();
//}
//
//void
//CacheCntlrDonuts::sendByWriteBuffer(const WriteBufferEntry& entry)
//{
//   // FIXME: e se for usar write-buffer nos níveis intermediários da cache??
//   printf("Sending via write-buffer [ %lx ] (time: %lu)...\n", entry.getAddress(), getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD).getNS());
//
//   Byte data_buf[getCacheBlockSize()];
//   processPersist(entry.getAddress(), data_buf);
//}
//
//void
//CacheCntlrDonuts::produceWriteBuffer()
//{
//   while (!m_dirty_blocks.empty() && m_buffer.size() < 16)
//   {
//      m_buffer.push(m_dirty_blocks.front());
//      m_dirty_blocks.pop();
//
//      auto entry = m_buffer.back();
//      auto cache_block = entry.first;
//      auto address = m_master->m_cache->tagToAddress(cache_block->getTag());
//
//      printf("Time: %lu | Commiting [ %lx ]...\n", getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD).getNS(), address);
//
//      Byte data_buf[getCacheBlockSize()];
//      processCommit(address, data_buf);
//   }
//}
//
//void
//CacheCntlrDonuts::consumeWriteBuffer()
//{
//   auto now = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
//   printf("Time: %lu...\n", now.getNS());
//   while (!m_buffer.empty() && now > m_buffer.front().second)
//   {
//      auto entry = m_buffer.front();
//      auto cache_block = entry.first;
//      auto address = m_master->m_cache->tagToAddress(cache_block->getTag());
//
//      printf("Time: %lu | Persisted via writebuffer [ %lx ]...\n", now.getNS(), address);
//
//      Byte data_buf[getCacheBlockSize()];
//      processPersist(address, data_buf);
//      m_buffer.pop();
//   }
//}
//
//void
//CacheCntlrDonuts::flushAll()
//{
//   auto now = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
////   auto delay = m_dirty_blocks.back().second - now;
//   while (!m_dirty_blocks.empty())
//   {
//      auto entry = m_buffer.front();
//      auto cache_block = entry.first;
//      auto address = m_master->m_cache->tagToAddress(cache_block->getTag());
//
//      printf("Time: %lu | Persisted via writebuffer [ %lx ]...\n", now.getNS(), address);
//
//      Byte data_buf[getCacheBlockSize()];
//      processCommit(address, data_buf);
//      processPersist(address, data_buf);
//      m_dirty_blocks.pop();
//   }
//}
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