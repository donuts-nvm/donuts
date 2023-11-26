#pragma once

#include "cache_cntlr_wb.h"
#include "epoch_cntlr.h"

namespace ParametricDramDirectoryMSI
{

class CacheCntlrDonuts : public CacheCntlrWrBuff
{
public:

   CacheCntlrDonuts(MemComponent::component_t mem_component,
                    String name,
                    core_id_t core_id,
                    MemoryManager *memory_manager,
                    AddressHomeLookup *tag_directory_home_lookup,
                    Semaphore *user_thread_sem,
                    Semaphore *network_thread_sem,
                    UInt32 cache_block_size,
                    CacheParameters &cache_params,
                    ShmemPerfModel *shmem_perf_model,
                    bool is_last_level_cache,
                    EpochCntlr *epoch_cntlr);

   virtual ~CacheCntlrDonuts();

   void checkpoint();

   EpochCntlr* getEpochCntlr() const { return m_epoch_cntlr; }

private:

   EpochCntlr* m_epoch_cntlr;
};

}