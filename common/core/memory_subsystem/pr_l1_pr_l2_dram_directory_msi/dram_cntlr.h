#pragma once

// Define to re-enable DramAccessCount
//#define ENABLE_DRAM_ACCESS_COUNT

#include <unordered_map>

#include "dram_perf_model.h"
#include "shmem_msg.h"
#include "shmem_perf.h"
#include "fixed_types.h"
#include "memory_manager_base.h"
#include "dram_cntlr_interface.h"
#include "subsecond_time.h"

class FaultInjector;

namespace PrL1PrL2DramDirectoryMSI
{
   class DramCntlr : public DramCntlrInterface
   {
      protected:
         // Added by Kleber Kruger to allow derived memory controller classes
         DramCntlr(MemoryManagerBase* memory_manager,
                   ShmemPerfModel* shmem_perf_model,
                   UInt32 cache_block_size,
                   DramPerfModel* dram_perf_model,
                   MemComponent::component_t mem_component);

         std::unordered_map<IntPtr, Byte*> m_data_map;
         DramPerfModel* m_dram_perf_model;
         FaultInjector* m_fault_injector;
         MemComponent::component_t m_mem_component;

         typedef std::unordered_map<IntPtr,UInt64> AccessCountMap;
         AccessCountMap* m_dram_access_count;
         UInt64 m_reads, m_writes;

         ShmemPerf m_dummy_shmem_perf;

         SubsecondTime runDramPerfModel(core_id_t requester, SubsecondTime time, IntPtr address, DramCntlrInterface::access_t access_type, ShmemPerf *perf);

         void addToDramAccessCount(IntPtr address, access_t access_type);
         void printDramAccessCount(void);

      private:
         DramCntlr(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size);

      public:
         ~DramCntlr() override;

         DramPerfModel* getDramPerfModel() { return m_dram_perf_model; }

         // Run DRAM performance model. Pass in begin time, returns latency
         boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf);
         boost::tuple<SubsecondTime, HitWhere::where_t> putDataToDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now);

         static DramCntlr* create(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size);
   };
}
