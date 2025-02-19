#pragma once

#include "../pr_l1_pr_l2_dram_directory_msi/nvm_cntlr.h"
#include "epoch_predictor.h"

namespace donuts
{

class LogRowBuffer final
{
public:
   explicit LogRowBuffer(core_id_t core_id,
                         UInt32 capacity, const SubsecondTime& flush_latency,
                         const SubsecondTime& insertion_latency = SubsecondTime::Zero());
   ~LogRowBuffer();

   SubsecondTime insert(IntPtr address, Byte* data_buf, UInt32 data_size);
   SubsecondTime flush();

   [[nodiscard]] UInt32 getCapacity() const { return m_buffer.capacity; }
   [[nodiscard]] UInt32 getSize() const { return m_buffer.size; }
   [[nodiscard]] UInt64 getNumFlushes() const { return m_flushes; }

private:
   struct
   {
      UInt32 size     = 0;
      UInt32 capacity = 0;
      std::unordered_map<IntPtr, Byte*> data;
   } m_buffer;

   UInt64 m_flushes;
   SubsecondTime m_insertion_latency;
   SubsecondTime m_flush_latency;
};

class NvmCntlr final : public PrL1PrL2DramDirectoryMSI::NvmCntlr
{
public:
   NvmCntlr(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size);

   ~NvmCntlr() override;

   boost::tuple<SubsecondTime, HitWhere::where_t>
   getDataFromDram(const IntPtr address, const core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf* perf) override
   {
      return getDataFromNvm(address, requester, data_buf, now, perf);
   }

   boost::tuple<SubsecondTime, HitWhere::where_t>
   putDataToDram(const IntPtr address, const core_id_t requester, Byte* data_buf, SubsecondTime now) override
   {
      return putDataToNvm(address, requester, data_buf, now);
   }

   boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now, ShmemPerf* perf) override;
   boost::tuple<SubsecondTime, HitWhere::where_t> putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now) override;
   boost::tuple<SubsecondTime, HitWhere::where_t> logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, const SubsecondTime& now) override;

private:
   struct LogPolicyState
   {
      LoggingPolicy value   = LoggingPolicy::LOGGING_DISABLED;
      LoggingPolicy current = LoggingPolicy::LOGGING_DISABLED;
   };

   LogPolicyState m_log_policy;
   std::unordered_map<LoggingPolicy, UInt64> m_num_logging;
   LogRowBuffer m_log_row_buffer;
   EpochPredictor m_epoch_predictor;

   static LoggingPolicy getLogPolicy();
   static UInt32 getLogRowBufferSize();
   static SubsecondTime getInsertionLatency();
   static SubsecondTime getFlushLatency();
};

}// namespace donuts

using NvmCntlrDonuts = donuts::NvmCntlr;
