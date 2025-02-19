#pragma once

#include "../parametric_dram_directory_msi/cache_cntlr.h"
#include "epoch_cntlr.h"

namespace donuts
{

class CacheCntlr final : public ParametricDramDirectoryMSI::CacheCntlr
{
public:
   enum class PersistencePolicy
   {
      SEQUENTIAL,
      FULLEST_FIRST,
      BALANCED
   };

   CacheCntlr(MemComponent::component_t mem_component,
              const String& name,
              core_id_t core_id,
              ParametricDramDirectoryMSI::MemoryManager* memory_manager,
              AddressHomeLookup* tag_directory_home_lookup,
              Semaphore* user_thread_sem,
              Semaphore* network_thread_sem,
              UInt32 cache_block_size,
              ParametricDramDirectoryMSI::CacheParameters& cache_params,
              ShmemPerfModel* shmem_perf_model,
              bool is_last_level_cache,
              EpochCntlr& epoch_cntlr);

   ~CacheCntlr() override;

   void checkpoint(CheckpointReason reason, UInt32 evicted_set_index) override;

private:
   EpochCntlr& m_epoch_cntlr;
   PersistencePolicy m_persistence_policy;

   void addDirtyBlocksFromSet(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const;
   std::queue<CacheBlockInfo*> selectDirtyBlocks(UInt32 evicted_set_index) const;

   void processCommit(IntPtr address, Byte* data_buf);

   void sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, IntPtr address, Byte* data_buf);

   void printCache(); // ONLY FOR DEBUG //

   static PersistencePolicy getPersistencePolicy();
};

}

using CacheCntlrDonuts = donuts::CacheCntlr;
