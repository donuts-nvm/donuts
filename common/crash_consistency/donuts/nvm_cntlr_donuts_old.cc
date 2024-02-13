//#include "nvm_cntlr_donuts.h"
//#include "simulator.h"
//#include "hooks_manager.h"
//#include "stats.h"
//#include "config.hpp"
//#include "epoch_manager.h"
//
//namespace PrL1PrL2DramDirectoryMSI
//{
//
//LogRowBuffer::LogRowBuffer(UInt32 capacity, const SubsecondTime& flush_latency, const SubsecondTime& insertion_latency) :
//    m_flushes(0), m_insertion_latency(insertion_latency), m_flush_latency(flush_latency)
//{
//   m_buffer.capacity = capacity;
//   m_buffer.size = 0;
////   printf("Size: %lu\n", m_buffer.data.size());
//
//   registerStatsMetric("log_row_buffer", 0, "flushes", &m_flushes);
//}
//
//LogRowBuffer::~LogRowBuffer() = default;
//
//SubsecondTime LogRowBuffer::insert(IntPtr address, Byte* data_buf, UInt32 data_size)
//{
//   SubsecondTime latency = m_insertion_latency;
//   if (m_buffer.size + data_size > m_buffer.capacity)
//      latency += flush();
//
//   m_buffer.data[address] = data_buf;
//   m_buffer.size += data_size;
////   printf("Size: %u | latency: %lu\n", m_buffer.size, latency.getNS());
//
//   return latency;
//}
//
//SubsecondTime LogRowBuffer::flush()
//{
//   m_flushes++;
////   printf("Flushes: %lu\n", m_flushes);
//   // FIXME: Escrever os dados na região de log da memória?
//
//   m_buffer.data.clear();
//   m_buffer.size = 0;
//
////   DramCntlr::runDramPerfModel(requester, now, address, LOG, &m_dummy_shmem_perf);
//   return m_flush_latency;
//}
//
//NvmCntlrDonuts::NvmCntlrDonuts(MemoryManagerBase* memory_manager, ShmemPerfModel* shmem_perf_model, UInt32 cache_block_size) :
//    NvmCntlr(memory_manager, shmem_perf_model, cache_block_size),
//    m_log_row_buffer(getLogRowBufferSize(), getFlushLatency(), getInsertionLatency()),
//    m_checkpoint_region(getCheckpointRegion())
//{
//   LOG_ASSERT_ERROR(getLogRowBufferSize() % getCacheBlockSize() == 0,
//                    "Log row buffer size must be a number divisible by the cache block size (%u %% %u)",
//                    getLogRowBufferSize(), getCacheBlockSize());
//
//   m_log_policy.value = getLogPolicy();
//   m_log_policy.current = m_log_policy.value == LogPolicy::LOGGING_HYBRID ? LOGGING_ON_WRITE : m_log_policy.value;
//
//   Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_END, _checkpoint, (UInt64) this);
//}
//
//NvmCntlrDonuts::~NvmCntlrDonuts() = default;
//
//boost::tuple<SubsecondTime, HitWhere::where_t>
//NvmCntlrDonuts::getDataFromNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now, ShmemPerf* perf)
//{
//   SubsecondTime nvm_latency, log_latency;
//   HitWhere::where_t hit_where;
//
//   if (m_log_policy.current == LogPolicy::LOGGING_ON_READ)
//      log_latency = processLogging(address, requester, data_buf, now);
//
//   boost::tie(nvm_latency, hit_where) = NvmCntlr::getDataFromNvm(address, requester, data_buf, now, perf);
//   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(nvm_latency + log_latency, m_hit_where);
//
//   return values;
//}
//
//boost::tuple<SubsecondTime, HitWhere::where_t>
//NvmCntlrDonuts::putDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
//{
//   SubsecondTime nvm_latency, log_latency;
//   HitWhere::where_t hit_where;
//
//   if (m_log_policy.current == LogPolicy::LOGGING_ON_WRITE)
//      log_latency = processLogging(address, requester, data_buf, now);
//
////   printf("putDataToNvm [ %lx ] (time: %lu)...\n", address, now.getNS());
//
//   boost::tie(nvm_latency, hit_where) = NvmCntlr::putDataToNvm(address, requester, data_buf, now);
//   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(nvm_latency + log_latency, m_hit_where);
//
//   return values;
//}
//
//boost::tuple<SubsecondTime, HitWhere::where_t>
//NvmCntlrDonuts::logDataToNvm(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
//{
//   createLogEntry(address, data_buf);
//
//   SubsecondTime latency = m_log_row_buffer.insert(address, data_buf, getCacheBlockSize());
////   printf("%-5s [%08lx] in %luns\n", "LOG", address, latency.getNS());
////   latency = DramCntlr::runDramPerfModel(requester, now, address, LOG, &m_dummy_shmem_perf);
////   printf("LOG 2 [%08lx] latency = %lu\n", address, latency.getNS());
//
//   ++m_logs;
//#ifdef ENABLE_DRAM_ACCESS_COUNT
//   addToDramAccessCount(address, LOG);
//#endif
////   MYLOG("L @ %08lx", address);
//
//   auto values = boost::tuple<SubsecondTime, HitWhere::where_t>(latency, m_hit_where);
//
//   return values;
//}
//
//void
//NvmCntlrDonuts::createLogEntry(IntPtr address, Byte* data_buf)
//{
//   m_log_map[address] = data_buf;
////   memcpy((void*) m_log_map[address], (void*) data_buf, getCacheBlockSize());
//}
//
//SubsecondTime
//NvmCntlrDonuts::processLogging(IntPtr address, core_id_t requester, Byte* data_buf, SubsecondTime now)
//{
//   SubsecondTime latency;
//   HitWhere::where_t hit_where;
//
//   boost::tie(latency, hit_where) = logDataToNvm(address, requester, data_buf, now);
//   return latency;
//}
//
//void
//NvmCntlrDonuts::checkpoint(UInt64 eid)
//{
////   IntPtr pc = Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache() >> m_checkpoint_region;
//   IntPtr pc = Sim()->getEpochManager()->getCommitedPC() >> m_checkpoint_region;
//   if (m_log_policy.value == NvmCntlrDonuts::LOGGING_HYBRID) {
//      m_log_policy.current = m_checkpoint_predictor.predict(pc) ? LogPolicy::LOGGING_ON_READ : LogPolicy::LOGGING_ON_WRITE;
//   }
//   printf("Running epoch from PC = [%lx] on MODE [%d]\n", pc, m_log_policy.current);
//}
//
////void NvmCntlrDonuts::startCheckpoint(IntPtr pc)
////{
//////   IntPtr pc = Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache() >> 4;
////   if (m_log_policy.value == NvmCntlrDonuts::LOGGING_HYBRID) {
////      m_log_policy.current = m_checkpoint_predictor.predict(pc) ? LogPolicy::LOGGING_ON_READ : LogPolicy::LOGGING_ON_WRITE;
////   }
////   printf("CHECKPOINT on MODE %d\n", m_log_policy.current);
////}
////
////void NvmCntlrDonuts::finishCheckpoint(IntPtr pc)
////{
////   SubsecondTime latency = m_log_row_buffer.flush();
////
////   if (m_log_policy.value == LogPolicy::LOGGING_HYBRID)
////   {
////      m_checkpoint_predictor.predictNext(pc, m_log_policy.current == LogPolicy::LOGGING_ON_WRITE,
////                                         SubsecondTime::Zero(), 0);
////   }
////
////   // FIXME: Invocar este método quando realizar um checkpoint. Além disso, o que fazer com essa latência?
////}
//
//NvmCntlrDonuts::log_policy_t
//NvmCntlrDonuts::getLogPolicy()
//{
//   String param = "perf_model/nvm/pim/log_policy";
//   String value = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getString(param) : "disabled";
//
//   if (value == "disabled")   return NvmCntlrDonuts::LOGGING_DISABLED;
//   if (value == "lor")        return NvmCntlrDonuts::LOGGING_ON_READ;
//   if (value == "low")        return NvmCntlrDonuts::LOGGING_ON_WRITE;
//   if (value == "hybrid")     return NvmCntlrDonuts::LOGGING_HYBRID;
//   if (value == "cmd")        return NvmCntlrDonuts::LOGGING_ON_COMMAND;
//
//   LOG_ASSERT_ERROR(false, "Unknown log policy: %s", value.c_str());
//}
//
//UInt32
//NvmCntlrDonuts::getLogRowBufferSize()
//{
//   String param = "perf_model/nvm/log_row_buffer/size";
//   return Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getInt(param) : 4096;
//}
//
//SubsecondTime
//NvmCntlrDonuts::getInsertionLatency()
//{
//   String param = "perf_model/nvm/log_row_buffer/insertion_latency";
//   float latency = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getFloat(param) : 0.0;
//
//   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
//}
//
//SubsecondTime
//NvmCntlrDonuts::getFlushLatency()
//{
//   String param = "perf_model/nvm/log_row_buffer/flush_latency";
//   float latency = Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getFloat(param) : 0.0;
//
//   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency));
//}
//
//UInt32
//NvmCntlrDonuts::getCheckpointRegion()
//{
//   String param = "donuts/checkpoint_region";
//   return Sim()->getCfg()->hasKey(param) ? Sim()->getCfg()->getInt(param) : 4;
//}
//
//}
