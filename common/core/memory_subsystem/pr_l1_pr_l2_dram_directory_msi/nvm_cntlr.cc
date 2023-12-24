#include "nvm_cntlr.h"
#include "nvm_perf_model.h"
#include "stats.h"
#include "fault_injection.h"

#if 0
   extern Lock iolock;
#  include "core_manager.h"
#  include "simulator.h"
#  define MYLOG(...) { ScopedLock l(iolock); fflush(stdout); printf("[%s] %d%cdr %-25s@%3u: ", itostr(getShmemPerfModel()->getElapsedTime()).c_str(), getMemoryManager()->getCore()->getId(), Sim()->getCoreManager()->amiUserThread() ? '^' : '_', __FUNCTION__, __LINE__); printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#else
#  define MYLOG(...) {}
#endif

namespace PrL1PrL2DramDirectoryMSI
{

NvmCntlr::NvmCntlr(MemoryManagerBase* memory_manager,
                   ShmemPerfModel* shmem_perf_model,
                   UInt32 cache_block_size) :
    DramCntlr(memory_manager, shmem_perf_model, cache_block_size,
              NvmPerfModel::createNvmPerfModel(memory_manager->getCore()->getId(), cache_block_size),
              MemComponent::NVM),
    m_logs(0)
{
   // FIXME: Change to nvm instead dram?
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "logs", &m_logs); // Added by Kleber Kruger
}

NvmCntlr::~NvmCntlr() = default;

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf* perf)
{
   return DramCntlr::getDataFromDram(address, requester, data_buf, now, perf);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
{
   return DramCntlr::putDataToDram(address, requester, data_buf, now);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
{
   if (Sim()->getFaultinjectionManager())
   {
      if (m_log_map[address] == NULL)
      {
         LOG_PRINT_ERROR("Data Buffer does not exist");
      }
      memcpy((void*) m_log_map[address], (void*) data_buf, getCacheBlockSize());

      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into data_buf instead
      if (m_fault_injector)
         m_fault_injector->postWrite(address, address, getCacheBlockSize(), (Byte*)m_log_map[address], now);
   }

   SubsecondTime nvm_access_latency = runDramPerfModel(requester, now, address, LOG, &m_dummy_shmem_perf);

   ++m_logs;
#ifdef ENABLE_DRAM_ACCESS_COUNT
   addToDramAccessCount(address, LOG);
#endif
   MYLOG("L @ %08lx", address);

   return boost::tuple<SubsecondTime, HitWhere::where_t>(nvm_access_latency, m_hit_where);
}

}