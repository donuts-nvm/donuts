#include "cache_cntlr.h"
#include "config.hpp"
#include "hooks_manager.h"
#include "last_level_cache.h"
#include "memory_manager.h"

namespace donuts
{

CacheCntlr::CacheCntlr(const MemComponent::component_t mem_component,
                       const String& name,
                       const core_id_t core_id,
                       ParametricDramDirectoryMSI::MemoryManager* memory_manager,
                       AddressHomeLookup* tag_directory_home_lookup,
                       Semaphore* user_thread_sem,
                       Semaphore* network_thread_sem,
                       const UInt32 cache_block_size,
                       ParametricDramDirectoryMSI::CacheParameters& cache_params,
                       ShmemPerfModel* shmem_perf_model,
                       const bool is_last_level_cache,
                       EpochCntlr& epoch_cntlr) :
    ParametricDramDirectoryMSI::CacheCntlr(mem_component, name, core_id, memory_manager, tag_directory_home_lookup,
                                           user_thread_sem, network_thread_sem, cache_block_size, cache_params,
                                           shmem_perf_model, is_last_level_cache),
    m_epoch_cntlr(epoch_cntlr),
    m_persistence_policy(getPersistencePolicy())
{
   // printf("Cache %s (%p) | CacheCntlr (%p) | Core %u-%u\n", getCache()->getName().c_str(), getCache(), this, m_core_id_master, m_core_id);

   if (is_last_level_cache)
   {
      LOG_ASSERT_ERROR(!m_cache_writethrough, "%s does not support write-through for the last-level cache (LLC)", Sim()->getProjectName());

      // TODO: Implement for non-unified LLC... Use a Master Controller?
      LOG_ASSERT_ERROR(m_core_id_master == 0, "%s does not yet support a non-unified last-level cache (LLC)", Sim()->getProjectName());

      if (isMasterCache())
      {
         Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT,
                                                [this](UInt64) {
                                                   const auto last_index = dynamic_cast<LastLevelCache*>(m_master->m_cache)->getLastInsertedIndex();
                                                   checkpoint(CheckpointReason::PERIODIC_TIME, last_index);
                                                });

         Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS,
                                                [this](UInt64) {
                                                   const auto last_index = dynamic_cast<LastLevelCache*>(m_master->m_cache)->getLastInsertedIndex();
                                                   checkpoint(CheckpointReason::PERIODIC_INSTRUCTIONS, last_index);
                                                });
      }
   }
}

CacheCntlr::~CacheCntlr() = default;

void CacheCntlr::checkpoint(const CheckpointReason reason, const UInt32 evicted_set_index)
{
   if (auto dirty_blocks = selectDirtyBlocks(evicted_set_index); !dirty_blocks.empty())
   {
      printf("ANTES:\n");
      printCache();

      printf("[g:%lu s:%lu u:%lu]: EPOCA %lu persistindo %lu blocos\n",
             stats::getGlobalTime().getNS(), stats::getSimTime(0).getNS(), stats::getUserTime(0).getNS(),
             m_epoch_cntlr.getCurrentEID(), dirty_blocks.size());

      while (!dirty_blocks.empty())
      {
         const auto* cache_block = dirty_blocks.front();
         const auto address      = m_master->m_cache->tagToAddress(cache_block->getTag());
         Byte data_buf[getCacheBlockSize()];

         printf("[%lu]: Commitando endereco: [%lx] (%c) (tid=%lu)\n", stats::getSimTime(0).getNS(), address, cache_block->getCStateString(), syscall(SYS_gettid));
         processCommit(address, data_buf);
         dirty_blocks.pop();
      }

      m_epoch_cntlr.commit(reason);
      printf("CHECKPOINT TERMINOU COM %lu %lu\n", getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD).getNS(), stats::getSimTime(0).getNS());

      printf("DEPOIS:\n");
      printCache();
   }
}

void CacheCntlr::addDirtyBlocksFromSet(std::queue<CacheBlockInfo*>& dirty_blocks, const UInt32 set_index) const
{
   for (UInt32 way = 0; way < m_master->m_cache->getAssociativity(); way++)
   {
      if (auto* block_info = m_master->m_cache->peekBlock(set_index, way); block_info && block_info->isDirty())
         dirty_blocks.push(block_info);
   }
}

/**
 * Select the dirty blocks from the cache according to the persistence policy.
 * TODO: When non-unified cache, you must get all the dirty cache blocks that will be on other controllers.
 *
 * @param evicted_set_index
 * @return a sorted queue containing all the dirty blocks
 */
std::queue<CacheBlockInfo*>
CacheCntlr::selectDirtyBlocks(const UInt32 evicted_set_index) const
{
   LOG_ASSERT_ERROR(m_persistence_policy != PersistencePolicy::BALANCED, "Persistence policy not yet implemented");

   std::queue<CacheBlockInfo*> dirty_blocks;
   std::vector<std::pair<UInt32, double>> other_sets;

   addDirtyBlocksFromSet(dirty_blocks, evicted_set_index);

   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
   {
      if (i == evicted_set_index) continue;
      if (const auto used = m_persistence_policy == PersistencePolicy::FULLEST_FIRST ? m_master->m_cache->getSetCapacityUsed(i) : 1;
          used > 0)
      {
         other_sets.emplace_back(i, used);
      }
   }

   if (m_persistence_policy == PersistencePolicy::FULLEST_FIRST)
   {
      std::ranges::sort(other_sets.begin(), other_sets.end(), [](const auto& a, const auto& b) {
         return a.second > b.second;
      });
   }

   for (const auto set_index: other_sets | std::views::keys)
   {
      addDirtyBlocksFromSet(dirty_blocks, set_index);
   }

   return dirty_blocks;
}

// void CacheCntlr::processCommitReq(const IntPtr address, Byte* data_buf)
// {
//    sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT_REQ, address, data_buf);
//    updateCacheBlock(address, CacheState::SHARED, ParametricDramDirectoryMSI::Transition::COHERENCY, data_buf, ShmemPerfModel::_SIM_THREAD);
// }
//
// void CacheCntlr::processCommitRep(const IntPtr address, Byte* data_buf)
// {
//    // FIXME: Should I use USER_THREAD or SIM_THREAD to update the cache block?
//    sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT_REP, address, nullptr);
// }

void CacheCntlr::processCommit(const IntPtr address, Byte* data_buf)
{
   updateCacheBlock(address, CacheState::SHARED, ParametricDramDirectoryMSI::Transition::COHERENCY, data_buf, ShmemPerfModel::_SIM_THREAD);
   sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT_REQ, address, data_buf);
}

void CacheCntlr::sendMsg(const PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, const IntPtr address, Byte* data_buf)
{
   getMemoryManager()->sendMsg(msg_type, MemComponent::LAST_LEVEL_CACHE, MemComponent::TAG_DIR,
                               m_core_id_master, getHome(address), /* requester and receiver */
                               address, data_buf, data_buf != nullptr ? getCacheBlockSize() : 0,
                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_SIM_THREAD);
}

CacheCntlr::PersistencePolicy
CacheCntlr::getPersistencePolicy()
{
   const auto param = "donuts/persistence_policy";
   const auto value = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "sequential";

   if (value == "sequential") return PersistencePolicy::SEQUENTIAL;
   if (value == "fullest_first") return PersistencePolicy::FULLEST_FIRST;
   if (value == "balanced") return PersistencePolicy::BALANCED;

   LOG_ASSERT_ERROR(false, "Persistence policy not found: %", value.c_str());
}

/********************************************************************************
 * ONLY FOR DEBUG!
 ********************************************************************************/
void CacheCntlr::printCache()
{
   printf("============================================================\n");
   printf("Cache %s (%.2f%%) (%p)\n", getCache()->getName().c_str(), getCache()->getCapacityUsed() * 100, getCache());
   printf("------------------------------------------------------------\n");
   for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
   {
      printf("%s%4d  ", j == 0 ? "    " : "", j);
   }
   printf("\n------------------------------------------------------------\n");
   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
   {
      printf("%4d ", i);
      for (UInt32 j = 0; j < m_master->m_cache->getAssociativity(); j++)
      {
         const auto cache_block = m_master->m_cache->peekBlock(i, j);
         const auto state       = cache_block->getCState() != CacheState::INVALID ? cache_block->getCStateString() : ' ';
         printf("[%c %lu] ", state, cache_block->getEpochID());
      }
      printf("= %.1f\n", m_master->m_cache->getSetCapacityUsed(i) * 100);
   }
   printf("============================================================\n");
}

}// namespace donuts