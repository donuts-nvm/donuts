#include "nvm_perf_model.h"
#include "config.hpp"
#include "nvm_perf_model_constant.h"
#include "nvm_perf_model_normal.h"
#include "nvm_perf_model_readwrite.h"
#include "nvm_perf_model_readwritelog.h"
#include "shmem_perf.h"
#include "simulator.h"

NvmPerfModel::NvmPerfModel(core_id_t core_id, UInt64 cache_block_size) :
    DramPerfModel(core_id, cache_block_size),
    m_nvm_bandwidth(8 * Sim()->getCfg()->getFloat("perf_model/dram/per_controller_bandwidth")),
    m_total_access_latency(SubsecondTime::Zero()),
    m_writebuffer_size(getWriteBufferSize()) {}

NvmPerfModel::~NvmPerfModel() = default;

NvmPerfModel*
NvmPerfModel::createNvmPerfModel(core_id_t core_id, UInt32 cache_block_size)
{
   String type = Sim()->getCfg()->getString("perf_model/dram/type");

   if (type == "constant")
   {
      return new NvmPerfModelConstant(core_id, cache_block_size);
   }
   if (type == "readwrite")
   {
      return new NvmPerfModelReadWrite(core_id, cache_block_size);
   }
   if (type == "readwritelog")
   {
      return new NvmPerfModelReadWriteLog(core_id, cache_block_size);
   }
   if (type == "normal")
   {
      return new NvmPerfModelNormal(core_id, cache_block_size);
   }

   LOG_PRINT_ERROR("Invalid NVM model type %s", type.c_str());
}

SubsecondTime
NvmPerfModel::getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
                               DramCntlrInterface::access_t access_type, ShmemPerf* perf)
{
   // pkt_size is in 'Bytes'
   // m_nvm_bandwidth is in 'Bits per clock cycle'
   if ((!m_enabled) || (requester >= (core_id_t) Config::getSingleton()->getApplicationCores()))
   {
      return SubsecondTime::Zero();
   }

   SubsecondTime processing_time = m_nvm_bandwidth.getRoundedLatency(8 * pkt_size);// bytes to bits
   SubsecondTime queue_delay     = computeQueueDelay(pkt_time, processing_time, requester, access_type);
   if ((access_type == DramCntlrInterface::WRITE || access_type == DramCntlrInterface::LOG) && m_writebuffer_size > 1)
   {// FIXME: remove this condition when upgrading to the WriteBufferCntlr model
      queue_delay = queue_delay / m_writebuffer_size;
   }
   SubsecondTime access_cost    = computeAccessCost(access_type);
   SubsecondTime access_latency = queue_delay + processing_time + access_cost;

   //   if (access_type == DramCntlrInterface::WRITE) printf("(%lu + %lu + %lu) = %lu\n", processing_time.getNS(), access_cost.getNS(), queue_delay.getNS(), access_latency.getNS());

   // FIXME: use ShmemPerf::NVM_QUEUE, NVM_BUS and NVM_DEVICE?
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
NvmPerfModel::computeAccessCost(DramCntlrInterface::access_t access_type)
{
   return access_type == DramCntlrInterface::READ  ? getReadCost() :
          access_type == DramCntlrInterface::WRITE ? getWriteCost() :
                                                     getLogCost();
}

std::pair<SubsecondTime, SubsecondTime>
NvmPerfModel::getDefaultLatencies()
{
   if (DramCntlrInterface::getTechnology() == DramCntlrInterface::PCM)
      return std::make_pair<>(SubsecondTime::NS(50), SubsecondTime::NS(500));
   if (DramCntlrInterface::getTechnology() == DramCntlrInterface::STT_RAM)
      return std::make_pair<>(SubsecondTime::NS(10), SubsecondTime::NS(50));
   if (DramCntlrInterface::getTechnology() == DramCntlrInterface::MEMRISTOR)
      return std::make_pair<>(SubsecondTime::NS(10), SubsecondTime::NS(10));
   if (DramCntlrInterface::getTechnology() == DramCntlrInterface::RERAM)
      return std::make_pair<>(SubsecondTime::NS(10), SubsecondTime::NS(50));
   if (DramCntlrInterface::getTechnology() == DramCntlrInterface::INTEL_OPTANE)
      return std::make_pair<>(SubsecondTime::NS(90), SubsecondTime::NS(300));

   return std::make_pair<>(SubsecondTime::Zero(), SubsecondTime::Zero());
}

SubsecondTime
NvmPerfModel::getReadLatency()
{
   float latency = Sim()->getCfg()->hasKey("perf_model/dram/read_latency") ?
                         Sim()->getCfg()->getFloat("perf_model/dram/read_latency") :
                         Sim()->getCfg()->getFloat("perf_model/dram/latency");

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

SubsecondTime
NvmPerfModel::getWriteLatency()
{
   float latency = Sim()->getCfg()->hasKey("perf_model/dram/write_latency") ?
                         Sim()->getCfg()->getFloat("perf_model/dram/write_latency") :
                         Sim()->getCfg()->getFloat("perf_model/dram/latency");

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

SubsecondTime
NvmPerfModel::getLogLatency()
{
   if (Sim()->getCfg()->hasKey("perf_model/dram/log_latency"))
   {
      float latency = Sim()->getCfg()->getFloat("perf_model/dram/log_latency");
      return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
   }
   return getWriteLatency();
}

/**
 * FIXME: Remove-it updating to WriteBufferCntlr model.
 * @return get write buffer size of the LLC
 */
UInt32
NvmPerfModel::getWriteBufferSize()
{
   auto cache_levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
   String last_cache = cache_levels == 1 ? "l1_dcache" : "l" + String(std::to_string(cache_levels).c_str()) + "_cache";
   const String key  = "perf_model/" + last_cache + "/writebuffer/num_entries";
   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 1;
}
