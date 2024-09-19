#ifndef __NVM_CNTLR_INTERFACE_H
#define __NVM_CNTLR_INTERFACE_H

#include "dram_cntlr_interface.h"

class NvmCntlrInterface : public DramCntlrInterface
{
protected:
   MemoryManagerBase* m_memory_manager;
   ShmemPerfModel* m_shmem_perf_model;
   UInt32 m_cache_block_size;

   UInt32 getCacheBlockSize() { return m_cache_block_size; }
   MemoryManagerBase* getMemoryManager() { return m_memory_manager; }
   ShmemPerfModel* getShmemPerfModel() { return m_shmem_perf_model; }

public:
   typedef enum
   {
      READ = 0,
      WRITE,
      LOG, // Added by Kleber Kruger (for NVM checkpoint projects)
      NUM_ACCESS_TYPES
   } access_t;

   // Added by Kleber Kruger
   typedef enum
   {
      UNKNOWN,
      DRAM,
      NVM,
      HYBRID
   } technology_t;

   NvmCntlrInterface(MemoryManagerBase* memoryManager, ShmemPerfModel* shmemPerfModel, UInt32 cacheBlockSize, MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size) :
       DramCntlrInterface(memoryManager, shmemPerfModel, cacheBlockSize), m_memory_manager(memory_manager), m_shmem_perf_model(shmem_perf_model), m_cache_block_size(cache_block_size)
   {}
   virtual ~NvmCntlrInterface() {}

   virtual boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf) = 0;
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> putDataToDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) = 0;
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf) = 0;
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) = 0;

   void handleMsgFromTagDirectory(core_id_t sender, PrL1PrL2DramDirectoryMSI::ShmemMsg* shmem_msg);

   // Added by Kleber Kruger
   static NvmCntlrInterface::technology_t getTechnology();
};

#endif // __NVM_CNTLR_INTERFACE_H
