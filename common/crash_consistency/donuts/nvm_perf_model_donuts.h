#ifndef NVM_PERF_MODEL_DONUTS_H
#define NVM_PERF_MODEL_DONUTS_H

#include "nvm_perf_model_constant.h"

class NvmPerfModelDonuts : public NvmPerfModelConstant
{
private:

   struct {
      bool enabled;
      UInt32 size;
   } m_write_buffer{};

   static bool isWriteBufferEnabled();
   static UInt32 getWriteBufferSize();

public:

   NvmPerfModelDonuts(core_id_t core_id, UInt32 cache_block_size);
   ~NvmPerfModelDonuts() override;

   SubsecondTime getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
                                  DramCntlrInterface::access_t access_type, ShmemPerf *perf) override;
};

#endif // NVM_PERF_MODEL_DONUTS_H
