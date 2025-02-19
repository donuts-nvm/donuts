#include "dram_cntlr_interface.h"
#include "memory_manager.h"
#include "shmem_msg.h"
#include "shmem_perf.h"
#include "log.h"
#include "core_manager.h"
#include "config.hpp"

#include <unordered_set>

DramCntlrInterface::DramCntlrInterface(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, const UInt32 cache_block_size) :
    m_memory_manager(memory_manager),
    m_shmem_perf_model(shmem_perf_model),
    m_cache_block_size(cache_block_size),
    m_write_queue_perf_model(DramWriteQueuePerfModel::create(memory_manager->getCore()->getId()))
{
   static int n = 0;
   printf("DramCntlrInterface %d [%p]\n", n++, this);
}

void DramCntlrInterface::handleMsgFromTagDirectory(core_id_t sender, PrL1PrL2DramDirectoryMSI::ShmemMsg* shmem_msg)
{
   PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t shmem_msg_type = shmem_msg->getMsgType();
   SubsecondTime msg_time = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD);
   shmem_msg->getPerf()->updateTime(msg_time);

   switch (shmem_msg_type)
   {
      case PrL1PrL2DramDirectoryMSI::ShmemMsg::DRAM_READ_REQ:
      {
         IntPtr address = shmem_msg->getAddress();
         Byte data_buf[getCacheBlockSize()];
         SubsecondTime dram_latency;
         HitWhere::where_t hit_where;

         boost::tie(dram_latency, hit_where) = getDataFromDram(address, shmem_msg->getRequester(), data_buf, msg_time, shmem_msg->getPerf());

         getShmemPerfModel()->incrElapsedTime(dram_latency, ShmemPerfModel::_SIM_THREAD);

         shmem_msg->getPerf()->updateTime(getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD),
            hit_where == HitWhere::DRAM_CACHE ? ShmemPerf::DRAM_CACHE : ShmemPerf::DRAM);

         printf("[%lu] - READ %lu\n", msg_time.getNS(), dram_latency.getNS());

         getMemoryManager()->sendMsg(PrL1PrL2DramDirectoryMSI::ShmemMsg::DRAM_READ_REP,
               MemComponent::DRAM, MemComponent::TAG_DIR,
               shmem_msg->getRequester() /* requester */,
               sender /* receiver */,
               address,
               data_buf, getCacheBlockSize(),
               hit_where, shmem_msg->getPerf(), ShmemPerfModel::_SIM_THREAD);
         break;
      }

      case PrL1PrL2DramDirectoryMSI::ShmemMsg::DRAM_WRITE_REQ:
      {
         auto [dram_latency, _] = putDataToDram(shmem_msg->getAddress(), shmem_msg->getRequester(), shmem_msg->getDataBuf(), msg_time);

         printf("[%lu] - WRITE %lu\n", msg_time.getNS(), dram_latency.getNS());

         if (m_write_queue_perf_model)
         {
            const SubsecondTime delay = m_write_queue_perf_model->sendAndCalculateDelay(shmem_msg->getAddress(), msg_time);
            printf("--- now: %lu | delay: %lu\n", msg_time.getNS(), delay.getNS());
            Sim()->getCoreManager()->getCoreFromID(shmem_msg->getRequester())->getPerformanceModel()->incrementElapsedTime(delay);
         }

         // If the write queue is disabled, DRAM latency is ignored on write

         break;
      }

      default:
         LOG_PRINT_ERROR("Unrecognized Shmem Msg Type: %u", shmem_msg_type);
         break;
   }
}

std::pair<DramCntlrInterface::technology_t, String>
DramCntlrInterface::getTechnology()
{
   static const std::unordered_set<String> nvm_technologies = {
      "pcm", "stt-ram", "memristor", "reram", "optane"
   };

   const String param = "perf_model/dram/technology";
   String value       = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "dram";

   if (value == "dram")
      return std::make_pair(technology_t::DRAM, value);
   if (value == "nvm" || nvm_technologies.contains(value))
      return std::make_pair(technology_t::NVM, value);

   LOG_PRINT_ERROR("Parameter [perf_model/dram/technology] is invalid");
}