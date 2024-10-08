#ifndef NVM_PERF_MODEL_H
#define NVM_PERF_MODEL_H

#include "dram_perf_model.h"

class NvmPerfModel : public DramPerfModel
{
public:
   NvmPerfModel(core_id_t core_id, UInt64 cache_block_size);
   ~NvmPerfModel() override;

   SubsecondTime getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
                                  DramCntlrInterface::access_t access_type, ShmemPerf* perf) override;

   static NvmPerfModel* createNvmPerfModel(core_id_t core_id, UInt32 cache_block_size);

   static std::pair<SubsecondTime, SubsecondTime> getDefaultLatencies();
   static SubsecondTime getReadLatency();
   static SubsecondTime getWriteLatency();
   static SubsecondTime getLogLatency();
   static UInt32 getWriteBufferSize();// FIXME: Update to write-buffer controller model

protected:
   ComponentBandwidth m_nvm_bandwidth;
   SubsecondTime m_total_access_latency;
   UInt32 m_writebuffer_size;// FIXME: Is it really necessary?, update it to write-buffer controller model

   SubsecondTime computeAccessCost(DramCntlrInterface::access_t access_type);

   virtual SubsecondTime computeQueueDelay(SubsecondTime pkt_time, SubsecondTime processing_time, core_id_t requester,
                                           DramCntlrInterface::access_t access_type)                    = 0;
   virtual void increaseQueueDelay(DramCntlrInterface::access_t access_type, SubsecondTime queue_delay) = 0;

   virtual SubsecondTime getReadCost()  = 0;
   virtual SubsecondTime getWriteCost() = 0;
   virtual SubsecondTime getLogCost()   = 0;
};

#endif /* NVM_PERF_MODEL_H */
