#include "cache_cntlr.h"
#include "config.hpp"
#include "hooks_manager.h"
#include "memory_manager.h"

namespace ParametricDramDirectoryMSI::donuts
{

CacheCntlr::CacheCntlr(const MemComponent::component_t mem_component,
                       const String& name,
                       const core_id_t core_id,
                       MemoryManager* memory_manager,
                       AddressHomeLookup* tag_directory_home_lookup,
                       Semaphore* user_thread_sem,
                       Semaphore* network_thread_sem,
                       const UInt32 cache_block_size,
                       CacheParameters& cache_params,
                       ShmemPerfModel* shmem_perf_model,
                       const bool is_last_level_cache,
                       EpochCntlr& epoch_cntlr) :
    ParametricDramDirectoryMSI::CacheCntlr(mem_component, name, core_id, memory_manager, tag_directory_home_lookup,
                                           user_thread_sem, network_thread_sem, cache_block_size, cache_params,
                                           shmem_perf_model, is_last_level_cache),
    m_epoch_cntlr(epoch_cntlr),
    m_persistence_policy(getPersistencePolicy())
{
   printf("Cache %s (%p) | CacheCntlr (%p) | Core %u/%u\n", getCache()->getName().c_str(), getCache(), this, m_core_id, m_core_id_master);

   if (is_last_level_cache)
   {
      LOG_ASSERT_ERROR(!m_cache_writethrough, "DONUTS does not allow LLC write-through");

      // TODO: Implement for non-unified LLC... Use a Master Controller??
      LOG_ASSERT_ERROR(m_core_id_master == 0, "DONUTS does not allow non-unified LLC yet");

      const auto _timeout = [](const UInt64 self, const UInt64 eid) -> SInt64
      {
         reinterpret_cast<CacheCntlr*>(self)->checkpoint(CheckpointReason::PERIODIC_TIME, 0);
         return 0;
      };
      const auto _timeout_ins = [](const UInt64 self, const UInt64 eid) -> SInt64
      {
         reinterpret_cast<CacheCntlr*>(self)->checkpoint(CheckpointReason::PERIODIC_INSTRUCTIONS, 0);
         return 0;
      };

      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT, _timeout, reinterpret_cast<UInt64>(this));
      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS, _timeout_ins, reinterpret_cast<UInt64>(this));
   }
}

CacheCntlr::~CacheCntlr() = default;

void
CacheCntlr::checkpoint(const CheckpointReason reason, const UInt32 evicted_set_index)
{
   printf("CHECKPOINT by %s\n", CheckpointInfo::reasonToString(reason));

   if (auto dirty_blocks = selectDirtyBlocks(evicted_set_index); !dirty_blocks.empty())
   {
      while (!dirty_blocks.empty())
      {
         const auto* cache_block = dirty_blocks.front();
         const auto address      = m_master->m_cache->tagToAddress(cache_block->getTag());
         Byte data_buf[getCacheBlockSize()];

         processCommit(address, data_buf);
         processPersist(address, data_buf);

         dirty_blocks.pop();
      }
      m_epoch_cntlr.commit();
   }
}

void
CacheCntlr::addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, const UInt32 set_index) const
{
   for (UInt32 way = 0; way < m_master->m_cache->getAssociativity(); way++)
   {
      if (auto* block_info = m_master->m_cache->peekBlock(set_index, way); block_info && block_info->isDirty())
      {
         dirty_blocks.push(block_info);
      }
   }
}

/**
 * Select the dirty blocks from the cache according to persistence policy.
 * TODO: When it is a non-unified cache, you must get all the dirty cache blocks that will be on other controllers.
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

   addAllDirtyBlocks(dirty_blocks, evicted_set_index);

   for (UInt32 i = 0; i < m_master->m_cache->getNumSets(); i++)
   {
      if (i == evicted_set_index) continue;
      if (const auto used = m_persistence_policy == PersistencePolicy::FULLEST_FIRST ? m_master->m_cache->getSetCapacityUsed(i) : 1;
          used > 0) other_sets.emplace_back(i, used);
   }

   if (m_persistence_policy == PersistencePolicy::FULLEST_FIRST)
      std::sort(other_sets.begin(), other_sets.end(), [](const auto& a, const auto& b)
                { return a.second > b.second; });

   for (auto [set_index, _]: other_sets)
   {
      addAllDirtyBlocks(dirty_blocks, set_index);
   }

   return dirty_blocks;
}

void
CacheCntlr::processCommit(const IntPtr address, Byte* data_buf)
{
   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::COMMIT, MemComponent::TAG_DIR, address, nullptr);
   updateCacheBlock(address, CacheState::SHARED, Transition::COHERENCY, data_buf, ShmemPerfModel::_USER_THREAD);
}

void
CacheCntlr::processPersist(const IntPtr address, Byte* data_buf)
{
   sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::PERSIST, MemComponent::TAG_DIR, address, data_buf);
}

void
CacheCntlr::sendMsgTo(const PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, const MemComponent::component_t receiver_mem_component,
                      const IntPtr address, Byte* data_buf)
{
   getMemoryManager()->sendMsg(msg_type, MemComponent::LAST_LEVEL_CACHE, receiver_mem_component,
                               m_core_id_master, getHome(address), /* requester and receiver */
                               address, data_buf, data_buf != nullptr ? getCacheBlockSize() : 0,
                               HitWhere::UNKNOWN, &m_dummy_shmem_perf, ShmemPerfModel::_USER_THREAD);
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
         auto cache_block = m_master->m_cache->peekBlock(i, j);
         auto state       = cache_block->getCState() != CacheState::INVALID ? cache_block->getCStateString() : ' ';
         printf("[%c %lu] ", state, cache_block->getEpochID());
      }
      printf("= %.1f\n", m_master->m_cache->getSetCapacityUsed(i) * 100);
   }
   printf("============================================================\n");
}

}