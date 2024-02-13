#include "nvm_cntlr_donuts.h"
#include "simulator.h"
#include "hooks_manager.h"
#include "stats.h"
#include "config.hpp"
#include "epoch_manager.h"

#define MYLOG(...) {}

namespace PrL1PrL2DramDirectoryMSI
{

LogRowBuffer::LogRowBuffer(UInt32 capacity, const SubsecondTime& flush_latency, const SubsecondTime& insertion_latency) :
    m_flushes(0), m_insertion_latency(insertion_latency), m_flush_latency(flush_latency)
{
   m_buffer.capacity = capacity;
   m_buffer.size = 0;

   registerStatsMetric("log_row_buffer", 0, "flushes", &m_flushes);
}

LogRowBuffer::~LogRowBuffer() = default;

SubsecondTime LogRowBuffer::insert(IntPtr address, Byte* data_buf, UInt32 data_size)
{
   SubsecondTime latency = m_insertion_latency;
   if (m_buffer.size + data_size > m_buffer.capacity)
      latency += flush();

   m_buffer.data[address] = data_buf;
   m_buffer.size += data_size;

   return latency;
}

SubsecondTime LogRowBuffer::flush()
{
   m_flushes++;

   // FIXME: Escrever os dados na região de log da memória?

   m_buffer.data.clear();
   m_buffer.size = 0;

   return m_flush_latency;
}

NvmCntlrDonuts::NvmCntlrDonuts(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size) :
    NvmCntlr(memory_manager, shmem_perf_model, cache_block_size),
    m_log_row_buffer(getLogRowBufferSize(), getFlushLatency(), getInsertionLatency()),
    m_checkpoint_region_size(getCheckpointRegionSize()),
    m_logs_by_epoch(0)
{
   LOG_ASSERT_ERROR(getLogRowBufferSize() % getCacheBlockSize() == 0,
                    "Log row buffer size must be a number divisible by the cache block size (%u %% %u)",
                    getLogRowBufferSize(), getCacheBlockSize());

   m_log_policy.value = getLogPolicy();
   m_log_policy.current = m_log_policy.value == LogPolicy::LOGGING_HYBRID ? LOGGING_ON_WRITE : m_log_policy.value;

   if (m_log_policy.value == LogPolicy::LOGGING_HYBRID)
   {
//      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_START, _checkpoint, (UInt64) this);
   }

   const String filename = "sim.ckpts.csv";
   const String path = Sim()->getConfig()->getOutputDirectory() + "/" + filename.c_str();
   m_checkpoint_info = fopen(path.c_str(), "w");
   LOG_ASSERT_ERROR(m_checkpoint_info != nullptr, "Error on creating m_checkpoint_info file");
}

NvmCntlrDonuts::~NvmCntlrDonuts()
{
   fclose(m_checkpoint_info);
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlrDonuts::getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf* perf)
{
   SubsecondTime nvm_latency;
   SubsecondTime log_latency;
   HitWhere::where_t hit_where;

   if (m_log_policy.current == LogPolicy::LOGGING_ON_READ)
      log_latency = logDataToNvm(address, requester, data_buf, now).head;

   if (log_latency > SubsecondTime::Zero()) printf("Fechando LogRowBuffer in %lu\n", log_latency.getNS());

   boost::tie(nvm_latency, hit_where) = NvmCntlr::getDataFromNvm(address, requester, data_buf, now, perf);
   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(nvm_latency + log_latency, m_hit_where);

   return values;
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlrDonuts::putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
{
   SubsecondTime nvm_latency;
   SubsecondTime log_latency;
   HitWhere::where_t hit_where;

   if (m_log_policy.current == LogPolicy::LOGGING_ON_WRITE)
      log_latency = logDataToNvm(address, requester, data_buf, now).head;

   if (log_latency > SubsecondTime::Zero()) printf("Fechando LogRowBuffer in %lu\n", log_latency.getNS());

   boost::tie(nvm_latency, hit_where) = NvmCntlr::putDataToNvm(address, requester, data_buf, now);
   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(nvm_latency + log_latency, m_hit_where);

   return values;
}

boost::tuple<SubsecondTime, HitWhere::where_t>
NvmCntlrDonuts::logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
{
   createLogEntry(address, data_buf);

   SubsecondTime latency = m_log_row_buffer.insert(address, data_buf, getCacheBlockSize());

   ++m_logs_by_epoch;
   ++m_logs;
#ifdef ENABLE_DRAM_ACCESS_COUNT
   addToDramAccessCount(address, LOG);
#endif
   MYLOG("L @ %08lx", address);

   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(latency, m_hit_where);

   return values;
}

void
NvmCntlrDonuts::createLogEntry(IntPtr address, Byte* data_buf)
{
   m_log_map[address] = data_buf;
   //   memcpy((void*) m_log_map[address], (void*) data_buf, getCacheBlockSize());
}

//void
//NvmCntlrDonuts::checkpoint(UInt64 eid)
//{
//   m_logs_by_epoch = 0;
//
//   if (m_log_policy.value == LogPolicy::LOGGING_HYBRID)
//   {
////      IntPtr commited_pc = Sim()->getEpochManager()->getCommitedPC() >> m_checkpoint_region_size;
////      if (m_log_policy.current == LogPolicy::LOGGING_ON_READ && canLoRtoLoW())
////      {
////         m_checkpoint_predictor.remove(commited_pc);
////         m_log_policy.current = LogPolicy::LOGGING_ON_WRITE;
////      }
////      else if (m_log_policy.current == LogPolicy::LOGGING_ON_WRITE && canLoWtoLoR())
////      {
////         m_checkpoint_predictor.insert(commited_pc);
////         m_log_policy.current = LogPolicy::LOGGING_ON_READ;
////      }
//
////      IntPtr current_pc = Sim()->getEpochManager()->getSystemPC() >> m_checkpoint_region_size;
////      m_log_policy.current = m_checkpoint_predictor.predict(current_pc) ? LogPolicy::LOGGING_ON_READ : LogPolicy::LOGGING_ON_WRITE;
////      printf("Running epoch from PC = [%lx] on MODE [%d]\n", current_pc, m_log_policy.current);
//   }
//}

void
NvmCntlrDonuts::checkpoint(CheckpointEvent::type_t event_type, UInt64 num_dirty_blocks, float cache_capacity_used)
{
   UInt64 logs = m_log_policy.current == LOGGING_ON_WRITE ? num_dirty_blocks : m_logs_by_epoch;
   double write_amplification = (double) logs / (double) num_dirty_blocks;

   fprintf(m_checkpoint_info, "%d;%lu;%lu;%.0f;%.2f\n", event_type, logs, num_dirty_blocks, (double) logs * 64 / 1024, write_amplification);

   if (m_log_policy.value == LogPolicy::LOGGING_HYBRID)
   {

   }

   auto *epoch_manager = EpochManager::getInstance();
   UInt64 eid = epoch_manager->getSystemEID();
   epoch_manager->commit();
   epoch_manager->registerPersistedEID(eid);

   m_logs_by_epoch = 0;
}

NvmCntlrDonuts::log_policy_t
NvmCntlrDonuts::getLogPolicy()
{
   String param = "perf_model/nvm/pim/log_policy";
   String value = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "disabled";

   if (value == "disabled")   return NvmCntlrDonuts::LOGGING_DISABLED;
   if (value == "lor")        return NvmCntlrDonuts::LOGGING_ON_READ;
   if (value == "low")        return NvmCntlrDonuts::LOGGING_ON_WRITE;
   if (value == "hybrid")     return NvmCntlrDonuts::LOGGING_HYBRID;
   if (value == "cmd")        return NvmCntlrDonuts::LOGGING_ON_COMMAND;

   LOG_ASSERT_ERROR(false, "Unknown log policy: %s", value.c_str());
}

UInt32
NvmCntlrDonuts::getLogRowBufferSize()
{
   String param = "perf_model/nvm/log_row_buffer/size";
   return Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getInt(param) : 4096;
}

SubsecondTime
NvmCntlrDonuts::getInsertionLatency()
{
   String param = "perf_model/nvm/log_row_buffer/insertion_latency";
   float latency = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getFloat(param) : 0.0;

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

SubsecondTime
NvmCntlrDonuts::getFlushLatency()
{
   String param = "perf_model/nvm/log_row_buffer/flush_latency";
   float latency = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getFloat(param) : 0.0;

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
}

UInt32
NvmCntlrDonuts::getCheckpointRegionSize()
{
   String param = "donuts/checkpoint_region_size";
   return Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getInt(param) : 4;
}

}
