#ifndef NVM_CNTLR_DONUTS_H
#define NVM_CNTLR_DONUTS_H

#include "nvm_cntlr.h"
#include "checkpoint_predictor.h"
#include "checkpoint_event.h"

namespace PrL1PrL2DramDirectoryMSI
{

class LogRowBuffer
{
public:

   explicit LogRowBuffer(UInt32 capacity, const SubsecondTime& flush_latency,
                         const SubsecondTime& insertion_latency = SubsecondTime::Zero());
   virtual ~LogRowBuffer();

   SubsecondTime insert(IntPtr address, Byte* data_buf, UInt32 data_size);
   SubsecondTime flush();

   UInt32 getCapacity() const { return m_buffer.capacity; }
   UInt32 getSize() const { return m_buffer.size; }
   UInt64 getNumFlushes() const { return m_flushes; }

private:

   struct {
      UInt32 size;
      UInt32 capacity;
      std::unordered_map<IntPtr, Byte*> data;
   } m_buffer;

   UInt64 m_flushes;
   SubsecondTime m_insertion_latency;
   SubsecondTime m_flush_latency;
};

class NvmCntlrDonuts : public NvmCntlr
{
public:

   typedef enum log_policy_t
   {
      LOGGING_DISABLED = 0,
      LOGGING_ON_READ,
      LOGGING_ON_WRITE,
      LOGGING_HYBRID,
      LOGGING_ON_COMMAND,
      NUM_LOGGING_POLICIES = LOGGING_ON_COMMAND - 1
   } LogPolicy;

   NvmCntlrDonuts(MemoryManagerBase* memory_manager,
                  ShmemPerfModel* shmem_perf_model,
                  UInt32 cache_block_size);

   ~NvmCntlrDonuts() override;

   boost::tuple<SubsecondTime, HitWhere::where_t>
   getDataFromDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf) override {
      return getDataFromNvm(address, requester, data_buf, now, perf);
   }

   boost::tuple<SubsecondTime, HitWhere::where_t>
   putDataToDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) override {
      return putDataToNvm(address, requester, data_buf, now);
   }

   boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf) override;
   boost::tuple<SubsecondTime, HitWhere::where_t> putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) override;
   boost::tuple<SubsecondTime, HitWhere::where_t> logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) override;

//   void checkpoint(UInt64 eid);

   void checkpoint(CheckpointEvent::type_t event_type, UInt64 num_dirty_blocks, float cache_used);

private:

   struct {
      log_policy_t value;
      log_policy_t current;
   } m_log_policy;

   UInt64 m_num_logging[NUM_LOGGING_POLICIES];
   CheckpointPredictor m_checkpoint_predictor;

   LogRowBuffer m_log_row_buffer;
   UInt32 m_checkpoint_region_size;
   UInt64 m_logs_by_epoch;

   FILE* m_checkpoint_info;

   void createLogEntry(IntPtr address, Byte* data_buf);

   bool canLoRtoLoW();
   bool canLoWtoLoR();

   static log_policy_t getLogPolicy();
   static UInt32 getLogRowBufferSize();
   static SubsecondTime getInsertionLatency();
   static SubsecondTime getFlushLatency();
   static UInt32 getCheckpointRegionSize();

//   static SInt64 _checkpoint(UInt64 arg, UInt64 eid) { ((NvmCntlrDonuts *)arg)->checkpoint(eid); return 0; }
};

}

#endif /* NVM_CNTLR_DONUTS_H */
