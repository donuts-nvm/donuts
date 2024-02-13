#include "nvm_perf_model.h"
#include "nvm_perf_model_donuts.h"
#include "shmem_perf.h"
#include "config.hpp"
#include "simulator.h"

NvmPerfModelDonuts::NvmPerfModelDonuts(core_id_t core_id, UInt32 cache_block_size) :
    NvmPerfModelConstant(core_id, cache_block_size)
{
   m_write_buffer.enabled = isWriteBufferEnabled();
   m_write_buffer.size = getWriteBufferSize();
}

NvmPerfModelDonuts::~NvmPerfModelDonuts() = default;

SubsecondTime NvmPerfModelDonuts::getAccessLatency(SubsecondTime pkt_time, UInt64 pkt_size, core_id_t requester, IntPtr address,
                                     DramCntlrInterface::access_t access_type, ShmemPerf *perf)
{
   if (!m_write_buffer.enabled || access_type != DramCntlrInterface::WRITE)
   {
      return NvmPerfModelConstant::getAccessLatency(pkt_time, pkt_size, requester, address, access_type, perf);
   }

   // pkt_size is in 'Bytes'
   // m_nvm_bandwidth is in 'Bits per clock cycle'
   if ((!m_enabled) || (requester >= (core_id_t) Config::getSingleton()->getApplicationCores())) {
      return SubsecondTime::Zero();
   }

   SubsecondTime processing_time = m_nvm_bandwidth.getRoundedLatency(8 * pkt_size); // bytes to bits
   SubsecondTime queue_delay = computeQueueDelay(pkt_time, processing_time, requester, access_type) / m_write_buffer.size;
   SubsecondTime access_cost = computeAccessCost(access_type);
   SubsecondTime access_latency = queue_delay + processing_time + access_cost;

   printf("(%lu + %lu + %lu) = %lu\n", processing_time.getNS(), queue_delay.getNS(), access_cost.getNS(), access_latency.getNS());

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

bool NvmPerfModelDonuts::isWriteBufferEnabled()
{
   return Sim()->getCfg()->getBoolDefault("perf_model/llc/writebuffer/enabled", false);
}

UInt32 NvmPerfModelDonuts::getWriteBufferSize()
{
   const String key = "perf_model/llc/writebuffer/num_entries";
   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : UINT32_MAX;
}