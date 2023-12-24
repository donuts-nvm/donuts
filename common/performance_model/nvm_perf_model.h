#ifndef NVM_PERF_MODEL_H
#define NVM_PERF_MODEL_H

#include "dram_perf_model.h"

class NvmPerfModel : public DramPerfModel
{
public:

   NvmPerfModel(core_id_t core_id, UInt64 cache_block_size);
   virtual ~NvmPerfModel();

//   SubsecondTime getLogLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
//                               DramCntlrInterface::access_t access_type, ShmemPerf *perf);

   static NvmPerfModel *createNvmPerfModel(core_id_t core_id, UInt32 cache_block_size);

   static SubsecondTime getReadLatency();
   static SubsecondTime getWriteLatency();
   static SubsecondTime getLogLatency();
};

#endif /* NVM_PERF_MODEL_H */
