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
               shmem_perf_model, is_last_level_cache),
    m_writebuffer_enabled(isWriteBufferEnabled(cache_params)),
    m_writebuffer_cntlr(nullptr)
{
   if (isWriteBufferEnabled())
   {
      if (isMasterCache())
      {
         m_writebuffer_cntlr = new WriteBufferCntlr(this, cache_params.configName);
      }
      else
      {
         auto* master_cache_cntlr = getMemoryManager()->getCacheCntlrAt(m_core_id_master, mem_component);
         m_writebuffer_cntlr      = dynamic_cast<CacheCntlrWrBuff*>(master_cache_cntlr)->m_writebuffer_cntlr;
      }
   }
   //   printf("WriteBufferCntlr from %s | Core %u/%u (%p)\n", getCache()->getName().c_str(), m_core_id_master, m_core_id, m_writebuffer_cntlr);
}

CacheCntlrWrBuff::~CacheCntlrWrBuff()
{
   if (isWriteBufferEnabled() && isMasterCache())
   {
      delete m_writebuffer_cntlr;
   }
}

void
CacheCntlrWrBuff::writeCacheBlockAtNextLevel(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length, ShmemPerfModel::Thread_t thread_num, UInt64 eid)
{
   // The LLC already implements a DRAM-write strategy similar to a write-buffer using a queue-based model
   if (!isWriteBufferEnabled() || isLastLevel())
   {
      CacheCntlr::writeCacheBlockAtNextLevel(address, offset, data_buf, data_length, thread_num, eid);
   }
   else
   {  // Write the cache block in the next level cache via write-buffer
      SubsecondTime latency = m_writebuffer_cntlr->insert(address, offset, data_buf, data_length, thread_num, eid);
      getMemoryManager()->incrElapsedTime(latency, thread_num);
   }
}

void
CacheCntlrWrBuff::sendByWriteBuffer(const WriteBufferEntry& entry)
{
   m_next_cache_cntlr->writeCacheBlock(entry.getAddress(),
                                       entry.getOffset(),
                                       entry.getDataBuf(),
                                       entry.getDataLength(),
                                       entry.getThreadNum(),
                                       entry.getEpochID());
}

bool
CacheCntlrWrBuff::isWriteBufferEnabled(const CacheParameters& cache_params)
{
   return Sim()->getCfg()->getBoolDefault("perf_model/" + cache_params.configName + "/writebuffer/enabled", false);
}

}