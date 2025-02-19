#include "nvm_cntlr.h"
#include "config.hpp"
#include "simulator.h"
#include "stats.h"
#include "system_snapshot.h"

#include <cache_cntlr.h>

namespace donuts
{

LogRowBuffer::LogRowBuffer(const core_id_t core_id,
                           const UInt32 capacity, const SubsecondTime& flush_latency, const SubsecondTime& insertion_latency) :
    m_flushes(0), m_insertion_latency(insertion_latency), m_flush_latency(flush_latency)
{
   m_buffer.capacity = capacity;
   m_buffer.size     = 0;

   registerStatsMetric("nvm", core_id, "log_flushes", &m_flushes);
}

LogRowBuffer::~LogRowBuffer() = default;

SubsecondTime
LogRowBuffer::insert(const IntPtr address, Byte* data_buf, const UInt32 data_size)
{
   SubsecondTime latency = m_insertion_latency;
   if (m_buffer.size + data_size > m_buffer.capacity)
      latency += flush();

   m_buffer.data[address] = data_buf;
   m_buffer.size += data_size;

   return latency;
}

SubsecondTime
LogRowBuffer::flush()
{
   m_flushes++;

   // TODO: Write data in the log memory region?

   m_buffer.data.clear();
   m_buffer.size = 0;

   return m_flush_latency;
}

NvmCntlr::NvmCntlr(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, const UInt32 cache_block_size) :
    PrL1PrL2DramDirectoryMSI::NvmCntlr(memory_manager, shmem_perf_model, cache_block_size),
    m_log_policy({ getLogPolicy(), getLogPolicy() }),
    m_log_row_buffer(memory_manager->getCore()->getId(), getLogRowBufferSize(), getFlushLatency(), getInsertionLatency()),
    m_epoch_predictor(memory_manager->getCore()->getId())
{
   registerStatsMetric("nvm", memory_manager->getCore()->getId(), "log_policy",
                       reinterpret_cast<UInt64*>(&m_log_policy.current));
}

NvmCntlr::~NvmCntlr() = default;

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::getDataFromNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now, ShmemPerf* perf)
{
   return PrL1PrL2DramDirectoryMSI::NvmCntlr::getDataFromNvm(address, requester, data_buf, now, perf);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::putDataToNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now)
{
   return PrL1PrL2DramDirectoryMSI::NvmCntlr::putDataToNvm(address, requester, data_buf, now);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlr::logDataToNvm(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now)
{
   return PrL1PrL2DramDirectoryMSI::NvmCntlr::logDataToNvm(address, requester, data_buf, now);
}

// void NvmCntlr::handleEpochBarrier(const IntPtr address, const core_id_t requester, Byte* data_buf, const SubsecondTime& now)
// {
//    static std::pair<UInt64, UInt64> last{0, 0};
//    const UInt64 current = stats::getDramAccesses(WRITE);
//    printf("Entre a FENCE %lu e %lu houve %lu blocos escritos na DRAM [%lu] [%lu]\n",
//           last.first, last.first + 1, current - last.second, stats::getGlobalTime().getNS(), now.getNS());
//    last.first = last.first + 1;
//    last.second = current;
//
//    printf("PERSISTINDO O ULTIMO ENDERECO: %lx %d! %lu\n", address, requester, current);
// }

LoggingPolicy
NvmCntlr::getLogPolicy()
{
   const auto key   = "perf_model/nvm/pim/log_policy";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getString(key) : "disabled";

   if (value == "disabled")
      return LoggingPolicy::LOGGING_DISABLED;
   if (value == "lor")
      return LoggingPolicy::LOGGING_ON_READ;
   if (value == "low")
      return LoggingPolicy::LOGGING_ON_WRITE;
   if (value == "hybrid")
      return LoggingPolicy::LOGGING_HYBRID;
   if (value == "cmd")
      return LoggingPolicy::LOGGING_ON_COMMAND;

   LOG_PRINT_ERROR("Unknown log policy: %s", value.c_str());
}

UInt32
NvmCntlr::getLogRowBufferSize()
{
   const auto key = "perf_model/nvm/log_row_buffer/size";
   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 4096;
}

SubsecondTime
NvmCntlr::getInsertionLatency()
{
   const auto key     = "perf_model/nvm/log_row_buffer/insertion_latency";
   const auto latency = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getFloat(key) : 0.0;

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<double>::NStoFS(latency));
}

SubsecondTime
NvmCntlr::getFlushLatency()
{
   const auto key     = "perf_model/nvm/log_row_buffer/flush_latency";
   const auto latency = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getFloat(key) : 0.0;

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<double>::NStoFS(latency));
}

}
