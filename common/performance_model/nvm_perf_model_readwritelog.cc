#include "nvm_perf_model_readwritelog.h"
#include "simulator.h"
#include "config.hpp"
#include "stats.h"
#include "shmem_perf.h"

NvmPerfModelReadWriteLog::NvmPerfModelReadWriteLog(core_id_t core_id, UInt32 cache_block_size) :
    NvmPerfModelReadWrite(core_id, cache_block_size),
    m_queue_model_log(nullptr),
    m_total_log_queueing_delay(SubsecondTime::Zero())
{
   String mem_technology = DramCntlrInterface::getTechnology() == DramCntlrInterface::HYBRID ? "nvm" : "dram";
   if (Sim()->getCfg()->getBool("perf_model/dram/queue_model/enabled")) {
      String queue_model_type = Sim()->getCfg()->getString("perf_model/dram/queue_model/type");
      SubsecondTime rounded_latency = m_nvm_bandwidth.getRoundedLatency(8 * cache_block_size); // bytes to bits

      m_queue_model_log = QueueModel::create(mem_technology + "-queue-log", core_id, queue_model_type, rounded_latency);
   }

   registerStatsMetric(mem_technology, core_id, "total-log-queueing-delay", &m_total_log_queueing_delay);
}

NvmPerfModelReadWriteLog::~NvmPerfModelReadWriteLog()
{
   if (m_queue_model_log)
   {
      delete m_queue_model_log;
      m_queue_model_log = nullptr;
   }
}

SubsecondTime
NvmPerfModelReadWriteLog::computeQueueDelay(SubsecondTime pkt_time, SubsecondTime processing_time, core_id_t requester,
                                            DramCntlrInterface::access_t access_type)
{
   SubsecondTime queue_delay = SubsecondTime::Zero();
   if (m_queue_model_read)
   {
      auto &queue = access_type == DramCntlrInterface::READ  ? m_queue_model_read  :
                                  access_type == DramCntlrInterface::WRITE ? m_queue_model_write :  m_queue_model_log;
      queue->computeQueueDelay(pkt_time, processing_time, requester);

      if (access_type == DramCntlrInterface::READ && m_shared_readwrite)
      {
         m_queue_model_write->computeQueueDelay(pkt_time, processing_time, requester);
//         m_queue_model_log->computeQueueDelay(pkt_time, processing_time, requester); // should we run in this queue too ???
      }
   }
   return queue_delay;
}

void
NvmPerfModelReadWriteLog::increaseQueueDelay(DramCntlrInterface::access_t access_type, SubsecondTime queue_delay)
{
   if (access_type == DramCntlrInterface::READ)
      m_total_read_queueing_delay += queue_delay;
   else if (access_type == DramCntlrInterface::WRITE)
      m_total_write_queueing_delay += queue_delay;
   else
      m_total_log_queueing_delay += queue_delay;
}
