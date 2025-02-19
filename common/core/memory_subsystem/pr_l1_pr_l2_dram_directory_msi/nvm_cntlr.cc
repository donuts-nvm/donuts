#include "nvm_cntlr.h"
#include "fault_injection.h"
#include "nvm_perf_model.h"
#include "stats.h"

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

NvmCntlr::NvmCntlr(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, const UInt32 cache_block_size) :
    DramCntlr(memory_manager, shmem_perf_model, cache_block_size,
              NvmPerfModel::createNvmPerfModel(memory_manager->getCore()->getId(), cache_block_size),
              MemComponent::DRAM),
    m_logs(0)
{
   registerStatsMetric("dram", memory_manager->getCore()->getId(), "logs", &m_logs);
}

NvmCntlr::~NvmCntlr() = default;

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::getDataFromNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now, ShmemPerf* perf)
{
   return DramCntlr::getDataFromDram(address, requester, data_buf, now, perf);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::putDataToNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now)
{
   return DramCntlr::putDataToDram(address, requester, data_buf, now);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::logDataToNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now)
{
   if (Sim()->getFaultinjectionManager())
   {
      LOG_ASSERT_ERROR(m_log_map[address] != nullptr, "Log Buffer does not exist");
      memcpy(m_log_map[address], data_buf, getCacheBlockSize());

      // NOTE: assumes error occurs in memory. If we want to model bus errors, insert the error into log_buf instead
      if (m_fault_injector)
         m_fault_injector->postWrite(address, address, getCacheBlockSize(), m_log_map[address], now);
   }

   const SubsecondTime nvm_access_latency = runDramPerfModel(requester, now, address, LOG, &m_dummy_shmem_perf);

   ++m_logs;
#ifdef ENABLE_DRAM_ACCESS_COUNT
   addToDramAccessCount(address, LOG);
#endif
   MYLOG("L @ %08lx", address);

   return { nvm_access_latency, HitWhere::DRAM };
}

}// namespace PrL1PrL2DramDirectoryMSI