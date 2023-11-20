#include "cache_cntlr_wb.h"

namespace ParametricDramDirectoryMSI
{

CacheCntlrWrBuff::CacheCntlrWrBuff(MemComponent::component_t mem_component,
                                   const String& name,
                                   core_id_t core_id,
                                   MemoryManager* memory_manager,
                                   AddressHomeLookup* tag_directory_home_lookup,
                                   Semaphore* user_thread_sem,
                                   Semaphore* network_thread_sem,
                                   UInt32 cache_block_size,
                                   CacheParameters& cache_params,
                                   ShmemPerfModel* shmem_perf_model,
                                   bool is_last_level_cache) :
    CacheCntlr(mem_component, name, core_id, memory_manager, tag_directory_home_lookup,
               user_thread_sem, network_thread_sem, cache_block_size, cache_params,
               shmem_perf_model, is_last_level_cache)
{
   m_writebuffer_cntlr = new WriteBufferCntlr(this, cache_params.configName);
}

CacheCntlrWrBuff::~CacheCntlrWrBuff()
{
   delete m_writebuffer_cntlr;
}

void
CacheCntlrWrBuff::writeCacheBlockAtNextLevel(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length, ShmemPerfModel::Thread_t thread_num)
{
   if (!isLastLevel())
   {
      SubsecondTime latency = m_writebuffer_cntlr->insert(address, offset, data_buf, data_length, thread_num);
      getMemoryManager()->incrElapsedTime(latency, thread_num);
   }
   else
   {
      SubsecondTime latency = sendDataToDram(address);
      getMemoryManager()->incrElapsedTime(latency, ShmemPerfModel::_USER_THREAD);
   }
}

}
