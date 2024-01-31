#ifndef NVM_PERF_MODEL_READWRITE_H
#define NVM_PERF_MODEL_READWRITE_H

#include "nvm_perf_model.h"
#include "queue_model.h"
#include "fixed_types.h"
#include "subsecond_time.h"
#include "dram_cntlr_interface.h"

class NvmPerfModelReadWrite : public NvmPerfModel
{
protected:

   QueueModel *m_queue_model_read;
   QueueModel *m_queue_model_write;
   SubsecondTime m_nvm_read_cost;
   SubsecondTime m_nvm_write_cost;
   SubsecondTime m_nvm_log_cost;

   bool m_shared_readwrite;

   SubsecondTime m_total_read_queueing_delay;
   SubsecondTime m_total_write_queueing_delay;

   SubsecondTime computeQueueDelay(SubsecondTime pkt_time, SubsecondTime processing_time, core_id_t requester,
                                   DramCntlrInterface::access_t access_type) override;
   void increaseQueueDelay(DramCntlrInterface::access_t access_type, SubsecondTime queue_delay) override;

   SubsecondTime getReadCost() override { return m_nvm_read_cost; }
   SubsecondTime getWriteCost() override { return m_nvm_write_cost; }
   SubsecondTime getLogCost() override { return m_nvm_log_cost; }

public:

   NvmPerfModelReadWrite(core_id_t core_id, UInt32 cache_block_size);
   virtual ~NvmPerfModelReadWrite();
};

#endif // NVM_PERF_MODEL_READWRITE_H
