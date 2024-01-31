#ifndef NVM_PERF_MODEL_READWRITELOG_H
#define NVM_PERF_MODEL_READWRITELOG_H

#include "nvm_perf_model_readwrite.h"
#include "queue_model.h"
#include "fixed_types.h"
#include "subsecond_time.h"
#include "dram_cntlr_interface.h"

class NvmPerfModelReadWriteLog : public NvmPerfModelReadWrite
{
private:
   QueueModel *m_queue_model_log;
   SubsecondTime m_total_log_queueing_delay;

   SubsecondTime computeQueueDelay(SubsecondTime pkt_time, SubsecondTime processing_time, core_id_t requester,
                                   DramCntlrInterface::access_t access_type) override;
   void increaseQueueDelay(DramCntlrInterface::access_t access_type, SubsecondTime queue_delay) override;

public:
   NvmPerfModelReadWriteLog(core_id_t core_id, UInt32 cache_block_size);
   virtual ~NvmPerfModelReadWriteLog();
};

#endif // NVM_PERF_MODEL_READWRITELOG_H
