#include "simulator.h"
#include "nvm_perf_model.h"
#include "nvm_perf_model_constant.h"
#include "nvm_perf_model_readwrite.h"
#include "nvm_perf_model_normal.h"
#include "config.hpp"
#include "shmem_perf.h"

NvmPerfModel::NvmPerfModel(core_id_t core_id, UInt64 cache_block_size)
    : DramPerfModel(core_id, cache_block_size) { }

NvmPerfModel::~NvmPerfModel() = default;

NvmPerfModel* NvmPerfModel::createNvmPerfModel(core_id_t core_id, UInt32 cache_block_size)
{
   String type = Sim()->getCfg()->getString("perf_model/dram/type");

   if (type == "constant")
   {
      return new NvmPerfModelConstant(core_id, cache_block_size);
   }
   else if (type == "readwrite")
   {
      return new NvmPerfModelReadWrite(core_id, cache_block_size);
   }
   else if (type == "normal")
   {
      return new NvmPerfModelNormal(core_id, cache_block_size);
   }

   LOG_PRINT_ERROR("Invalid NVM model type %s", type.c_str());
}

//SubsecondTime
//NvmPerfModel::getLogLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
//                            DramCntlrInterface::access_t access_type, ShmemPerf *perf)
//{
//   SubsecondTime queue_delay = SubsecondTime::Zero();
//   SubsecondTime processing_time = SubsecondTime::Zero();
//   SubsecondTime access_cost = getLogLatency();
//
//   perf->updateTime(pkt_time);
//   perf->updateTime(pkt_time + queue_delay, ShmemPerf::NVM_LOG_QUEUE);
//   perf->updateTime(pkt_time + queue_delay + processing_time, ShmemPerf::NVM_LOG_BUS);
//   perf->updateTime(pkt_time + queue_delay + processing_time + access_cost, ShmemPerf::NVM_DEVICE);
//
//   return queue_delay + processing_time + access_cost;
//}

SubsecondTime NvmPerfModel::getReadLatency()
{
   float latency = Sim()->getCfg()->hasKey("perf_model/dram/read_latency") ?
                   Sim()->getCfg()->getFloat("perf_model/dram/read_latency") :
                   Sim()->getCfg()->getFloat("perf_model/dram/latency");

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

SubsecondTime NvmPerfModel::getWriteLatency()
{
   float latency = Sim()->getCfg()->hasKey("perf_model/dram/write_latency") ?
                   Sim()->getCfg()->getFloat("perf_model/dram/write_latency") :
                   Sim()->getCfg()->getFloat("perf_model/dram/latency");

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

SubsecondTime NvmPerfModel::getLogLatency()
{
   if (Sim()->getCfg()->hasKey("perf_model/dram/log_latency"))
   {
      float latency = Sim()->getCfg()->getFloat("perf_model/dram/log_latency");
      return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
   }
   return getWriteLatency();
}
