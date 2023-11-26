#include "cache_cntlr_donuts.h"
#include "memory_manager.h"

#  define MYLOG(...) {}

namespace ParametricDramDirectoryMSI
{

CacheCntlrDonuts::CacheCntlrDonuts(MemComponent::component_t mem_component,
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
                                   EpochCntlr *epoch_cntlr) :
    CacheCntlrWrBuff(mem_component, name, core_id, memory_manager, tag_directory_home_lookup,
                     user_thread_sem, network_thread_sem, cache_block_size, cache_params,
                     shmem_perf_model, is_last_level_cache),
    m_epoch_cntlr(epoch_cntlr)
{
   printf("CacheCntlrDonuts %s [%p]\n", name.c_str(), m_epoch_cntlr);
}

CacheCntlrDonuts::~CacheCntlrDonuts() = default;

void CacheCntlrDonuts::checkpoint()
{

}

}