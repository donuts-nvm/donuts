#include "nvm_cntlr_interface.h"
#include "memory_manager.h"
#include "shmem_msg.h"
#include "shmem_perf.h"
#include "log.h"
#include "config.hpp"          // Added by Kleber Kruger
#include "nvm_cntlr.h"         // Added by Kleber Kruger
#include "nvm_cntlr_donuts.h"  // Added by Kleber Kruger

void NvmCntlrInterface::handleMsgFromTagDirectory(core_id_t sender, PrL1PrL2DramDirectoryMSI::ShmemMsg* shmem_msg)
{
   PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t shmem_msg_type = shmem_msg->getMsgType();
   SubsecondTime msg_time = getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD);
   shmem_msg->getPerf()->updateTime(msg_time);

   switch (shmem_msg_type)
   {
      case PrL1PrL2DramDirectoryMSI::ShmemMsg::DRAM_READ_REQ:
      case PrL1PrL2DramDirectoryMSI::ShmemMsg::NVM_READ_REQ:   // Added by Kleber Kruger
      {
         IntPtr address = shmem_msg->getAddress();
         Byte data_buf[getCacheBlockSize()];
         SubsecondTime dram_latency;
         HitWhere::where_t hit_where;

         boost::tie(dram_latency, hit_where) = getDataFromDram(address, shmem_msg->getRequester(), data_buf, msg_time, shmem_msg->getPerf());

         getShmemPerfModel()->incrElapsedTime(dram_latency, ShmemPerfModel::_SIM_THREAD);

         shmem_msg->getPerf()->updateTime(getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_SIM_THREAD),
                                          shmem_msg_type == PrL1PrL2DramDirectoryMSI::ShmemMsg::NVM_READ_REQ ? ShmemPerf::NVM : // This line was added by Kleber Kruger
                                                hit_where == HitWhere::DRAM_CACHE ? ShmemPerf::DRAM_CACHE : ShmemPerf::DRAM);

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
      case PrL1PrL2DramDirectoryMSI::ShmemMsg::NVM_WRITE_REQ:  // Added by Kleber Kruger
      {
         //         putDataToDram(shmem_msg->getAddress(), shmem_msg->getRequester(), shmem_msg->getDataBuf(), msg_time);
         auto values = putDataToDram(shmem_msg->getAddress(), shmem_msg->getRequester(), shmem_msg->getDataBuf(), msg_time);
         getShmemPerfModel()->incrElapsedTime(values.head, ShmemPerfModel::_SIM_THREAD);

         // DRAM latency is ignored on write

         break;
      }

      // Added by Kleber Kruger
      case PrL1PrL2DramDirectoryMSI::ShmemMsg::NVM_LOG_REQ:
      {
         auto *nvm_cntlr = dynamic_cast<PrL1PrL2DramDirectoryMSI::NvmCntlr*>(this);
         nvm_cntlr->logDataToNvm(shmem_msg->getAddress(), shmem_msg->getRequester(), shmem_msg->getDataBuf(), msg_time);

         // NVM Log latency is ignored on log

         break;
      }

      default:
         LOG_PRINT_ERROR("Unrecognized Shmem Msg Type: %u", shmem_msg_type);
         break;
   }
}

// TODO: Improve this method (check if technology is valid! use enum to set NVM technology?)
NvmCntlrInterface::technology_t
NvmCntlrInterface::getTechnology()
{
   String param = "perf_model/dram/technology";
   String technology = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "dram";

   if (technology == "dram")     return DRAM;
   if (technology == "nvm")      return NVM;
   if (technology == "hybrid")   return HYBRID;

   LOG_ASSERT_ERROR(false, "Parameter [perf_model/dram/technology] is unknown");
   return UNKNOWN;
}
