#include "nvm_perf_model_constant.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"
#include "stats.h"
#include "shmem_perf.h"

NvmPerfModelConstant::NvmPerfModelConstant(core_id_t core_id, UInt32 cache_block_size) :
    NvmPerfModel(core_id, cache_block_size),
    m_queue_model(nullptr),
    m_nvm_read_cost(NvmPerfModel::getReadLatency()),
    m_nvm_write_cost(NvmPerfModel::getWriteLatency()),
    m_nvm_log_cost(NvmPerfModel::getLogLatency()),
    m_total_queueing_delay(SubsecondTime::Zero())
{
   // FIXME: use "nvm" whenever the technology is Non-Volatile Memory
   String mem_technology = DramCntlrInterface::getTechnology() == DramCntlrInterface::HYBRID ? "nvm" : "dram";

   if (Sim()->getCfg()->getBool("perf_model/dram/queue_model/enabled"))
   {
      m_queue_model = QueueModel::create(mem_technology + "-queue", core_id,
                                         Sim()->getCfg()->getString("perf_model/dram/queue_model/type"),
                                         m_nvm_bandwidth.getRoundedLatency(8 * cache_block_size)); // bytes to bits
   }

   registerStatsMetric(mem_technology, core_id, "total-access-latency", &m_total_access_latency);
   registerStatsMetric(mem_technology, core_id, "total-queueing-delay", &m_total_queueing_delay);
}

NvmPerfModelConstant::~NvmPerfModelConstant()
{
   if (m_queue_model)
   {
      delete m_queue_model;
      m_queue_model = nullptr;
   }
}

SubsecondTime
NvmPerfModelConstant::computeQueueDelay(SubsecondTime pkt_time, SubsecondTime processing_time, core_id_t requester,
                                        DramCntlrInterface::access_t access_type)
{
   return m_queue_model ? m_queue_model->computeQueueDelay(pkt_time, processing_time, requester) : SubsecondTime::Zero();
}

void
NvmPerfModelConstant::increaseQueueDelay(DramCntlrInterface::access_t access_type, SubsecondTime queue_delay)
{
   m_total_queueing_delay += queue_delay;
}
