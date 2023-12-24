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
   SubsecondTime m_nvm_log_cost;
   SubsecondTime m_total_log_queueing_delay;

public:
   NvmPerfModelReadWriteLog(core_id_t core_id, UInt32 cache_block_size);
   ~NvmPerfModelReadWriteLog() override;

   SubsecondTime getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
                                  DramCntlrInterface::access_t access_type, ShmemPerf *perf) override;
};

#endif // NVM_PERF_MODEL_READWRITELOG_H
