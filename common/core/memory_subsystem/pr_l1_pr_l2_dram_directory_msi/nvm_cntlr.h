#pragma once

#include "dram_cntlr.h"

namespace PrL1PrL2DramDirectoryMSI
{

class NvmCntlr : public DramCntlr
{
public:
   NvmCntlr(MemoryManagerBase* memory_manager,
            ShmemPerfModel* shmem_perf_model,
            UInt32 cache_block_size);

   ~NvmCntlr() override;

   [[nodiscard]] DramPerfModel* getNvmPerfModel() const { return m_dram_perf_model; }

   virtual boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now, ShmemPerf* perf);
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now);
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now);

protected:
   UInt64 m_logs;
   std::unordered_map<IntPtr, Byte*> m_log_map;
};

}