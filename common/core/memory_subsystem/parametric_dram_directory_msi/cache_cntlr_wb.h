#pragma once

#include "cache_cntlr.h"
#include "write_buffer_cntlr.h"
#include "memory_manager.h"
#include "config.hpp"

namespace ParametricDramDirectoryMSI
{

class CacheCntlrWrBuff : public CacheCntlr
{
public:

   /**
    * Creates a CacheCntlrWrBuff.
    *
    * @param mem_component
    * @param name
    * @param core_id
    * @param memory_manager
    * @param tag_directory_home_lookup
    * @param user_thread_sem
    * @param network_thread_sem
    * @param cache_block_size
    * @param cache_params
    * @param shmem_perf_model
    * @param is_last_level_cache
    */
   CacheCntlrWrBuff(MemComponent::component_t mem_component,
                    const String& name,
                    core_id_t core_id,
                    MemoryManager* memory_manager,
                    AddressHomeLookup* tag_directory_home_lookup,
                    Semaphore* user_thread_sem,
                    Semaphore* network_thread_sem,
                    UInt32 cache_block_size,
                    CacheParameters& cache_params,
                    ShmemPerfModel* shmem_perf_model,
                    bool is_last_level_cache);

   /**
    * Destroys this CacheCntlrWrBuff.
    */
   ~CacheCntlrWrBuff() override;

   bool isWriteBufferEnabled() const { return m_writebuffer_enabled; }

   friend WriteBufferCntlr;

protected:

   bool m_writebuffer_enabled;
   WriteBufferCntlr* m_writebuffer_cntlr;

   /**
    * Writes the cache block at next level using a write buffer.
    *
    * @param address
    * @param offset
    * @param data_buf
    * @param data_length
    * @param thread_num
    */
   void writeCacheBlockAtNextLevel(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length, ShmemPerfModel::Thread_t thread_num, UInt64 eid) override;

   virtual void sendByWriteBuffer(const WriteBufferEntry &entry);

private:

   static bool isWriteBufferEnabled(const CacheParameters& cache_params);
};

} // namespace ParametricDramDirectoryMSI
