#include "nvm_perf_model.h"
#include "config.hpp"
#include "nvm_perf_model_constant.h"
#include "nvm_perf_model_normal.h"
#include "nvm_perf_model_readwrite.h"
#include "shmem_perf.h"
#include "simulator.h"

#include <optional>

NvmPerfModel::NvmPerfModel(const core_id_t core_id, const UInt64 cache_block_size) :
    DramPerfModel(core_id, cache_block_size),
    m_nvm_bandwidth(static_cast<float>(8 * Sim()->getCfg()->getFloat("perf_model/dram/per_controller_bandwidth"))),
    m_total_access_latency(SubsecondTime::Zero()) {}

NvmPerfModel::~NvmPerfModel() = default;

NvmPerfModel*
NvmPerfModel::createNvmPerfModel(const core_id_t core_id, const UInt32 cache_block_size)
{
   const String type = Sim()->getCfg()->getString("perf_model/dram/type");

   if (type == "constant")
   {
      return new NvmPerfModelConstant(core_id, cache_block_size);
   }
   if (type == "readwrite")
   {
      return new NvmPerfModelReadWrite(core_id, cache_block_size);
   }
   if (type == "normal")
   {
      return new NvmPerfModelNormal(core_id, cache_block_size);
   }

   LOG_PRINT_ERROR("Invalid NVM model type %s", type.c_str());
}

SubsecondTime
NvmPerfModel::getAccessLatency(SubsecondTime pkt_time, const UInt64 pkt_size, const core_id_t requester,
                               const IntPtr address, const DramCntlrInterface::access_t access_type, ShmemPerf* perf)
{
   // pkt_size is in 'Bytes'
   // m_nvm_bandwidth is in 'Bits per clock cycle'
   if (!m_enabled || requester >= static_cast<core_id_t>(Config::getSingleton()->getApplicationCores()))
      return SubsecondTime::Zero();

   const SubsecondTime processing_time = m_nvm_bandwidth.getRoundedLatency(8 * pkt_size);// bytes to bits
   const SubsecondTime queue_delay     = computeQueueDelay(pkt_time, processing_time, requester, access_type);
   const SubsecondTime access_cost     = computeAccessCost(access_type);
   const SubsecondTime access_latency  = queue_delay + processing_time + access_cost;

   perf->updateTime(pkt_time);
   perf->updateTime(pkt_time + queue_delay, ShmemPerf::DRAM_QUEUE);
   perf->updateTime(pkt_time + queue_delay + processing_time, ShmemPerf::DRAM_BUS);
   perf->updateTime(pkt_time + queue_delay + processing_time + access_cost, ShmemPerf::DRAM_DEVICE);

   // Update Memory Counters
   m_num_accesses++;
   m_total_access_latency += access_latency;
   increaseQueueDelay(access_type, queue_delay);

   return access_latency;
}

SubsecondTime
NvmPerfModel::computeAccessCost(const DramCntlrInterface::access_t access_type)
{
   return access_type == DramCntlrInterface::READ ? getReadCost() : getWriteCost();
}

SubsecondTime
NvmPerfModel::loadLatency(const String& param)
{
   const auto getLatencyValue = [param](const String& key) -> std::optional<float> {
      if (Sim()->getCfg()->hasKey(key + "/" + param)) return Sim()->getCfg()->getFloat(key + "/" + param);
      if (Sim()->getCfg()->hasKey(key + "/latency")) return Sim()->getCfg()->getFloat(key + "/latency");
      return std::nullopt;
   };

   const auto latency = getLatencyValue("perf_model/" + DramCntlrInterface::getTechnology().second)
                              .value_or(*getLatencyValue("perf_model/dram"));

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<double>::NStoFS(latency));
}
