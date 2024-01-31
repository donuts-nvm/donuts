#ifndef NVM_CNTLR_DONUTS_H
#define NVM_CNTLR_DONUTS_H

#include "nvm_cntlr.h"
#include "checkpoint_predictor.h"

namespace PrL1PrL2DramDirectoryMSI
{

class LogRowBuffer
{
public:

   explicit LogRowBuffer(UInt32 capacity, SubsecondTime flush_latency,
                         SubsecondTime insertion_latency = SubsecondTime::Zero());
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

   virtual ~NvmCntlrDonuts();

   inline virtual boost::tuple<SubsecondTime, HitWhere::where_t>
   getDataFromDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf) {
      return getDataFromNvm(address, requester, data_buf, now, perf);
   }

   inline virtual boost::tuple<SubsecondTime, HitWhere::where_t>
   putDataToDram(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now) {
      return putDataToNvm(address, requester, data_buf, now);
   }

   virtual boost::tuple<SubsecondTime, HitWhere::where_t> getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf *perf);
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now);
   virtual boost::tuple<SubsecondTime, HitWhere::where_t> logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now);

   void startCheckpoint(IntPtr pc);
   void finishCheckpoint(IntPtr pc);

   static log_policy_t getLogPolicy();

private:

   struct {
      log_policy_t value;
      log_policy_t current;
   } m_log_policy;

   UInt64 m_num_logging[NUM_LOGGING_POLICIES];
   CheckpointPredictor m_checkpoint_predictor;

   LogRowBuffer m_log_row_buffer;

   void createLogEntry(IntPtr address, Byte* data_buf);

   SubsecondTime processLogging(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now);
   bool canLoRtoLoW();
   bool canLoWtoLoR();

   static UInt32 getLogRowBufferSize();
   static SubsecondTime getInsertionLatency();
   static SubsecondTime getFlushLatency();
};

}

#endif /* NVM_CNTLR_DONUTS_H */
