#include "dram_write_queue_perf_model.h"
#include "config.hpp"
#include "dram_cntlr_interface.h"
#include "simulator.h"
#include "stats.h"

#include <memory>
#include <optional>

DramWriteQueuePerfModel::DramWriteQueuePerfModel(const core_id_t core_id) :
    m_time(SubsecondTime::Zero()),
    m_write_cost(loadWriteLatency()),
    m_enqueue_cost(loadEnqueueLatency()),
    m_num_entries(loadNumEntries()),
    m_merging(loadMerging()),
    m_burst_size(loadBurstSize()),
    m_total_latency(SubsecondTime::Zero()),
    m_total_queueing_delay(SubsecondTime::Zero()),
    m_num_accesses(0),
    m_num_insertions(0),
    m_num_overflows(0)
{
   registerStatsMetric("dram-write-queue", core_id, "total-accesses", &m_num_accesses);
   registerStatsMetric("dram-write-queue", core_id, "total-insertions", &m_num_insertions);
   registerStatsMetric("dram-write-queue", core_id, "total-overflows", &m_num_overflows);
   registerStatsMetric("dram-write-queue", core_id, "total-access-latency", &m_total_latency);
}

void DramWriteQueuePerfModel::consume(const SubsecondTime& now)
{
   while (!m_queue.empty() && now >= m_queue.front().second)
      m_queue.pop_front();
}

SubsecondTime
DramWriteQueuePerfModel::sendAndCalculateDelay(const IntPtr address, const SubsecondTime& write_msg_time)
{
   if (m_time < write_msg_time)
      m_time = write_msg_time;

   SubsecondTime delay = m_enqueue_cost;
   consume(m_time);

   auto same_address = [&](auto& e) {
      return e.first == address;
   };

   if (!m_merging || std::ranges::find_if(m_queue.rbegin(), m_queue.rend(), same_address) == m_queue.rend())
   {
      const SubsecondTime t_start = m_queue.empty() ? m_time : m_queue.back().second;
      if (m_queue.size() >= m_num_entries)
      {
         delay += m_queue.front().second - m_time;
         m_queue.pop_front();
         m_num_overflows++;
      }
      // m_queue.emplace_back(address, t_start + m_write_cost);
      const auto cost = m_num_insertions % m_burst_size == 0 ? m_write_cost : SubsecondTime::Zero();
      m_queue.emplace_back(address, t_start + cost);
      m_num_insertions++;

      print();
   }
   m_num_accesses++;
   m_time += delay;

   return delay;
}

void DramWriteQueuePerfModel::print() const
{
   UInt32 index = 0;
   printf("----------------------\n");
   for (const auto& [addr, time]: m_queue) printf("%4u [%12lX] -> (%lu)\n", index++, addr, time.getNS());
   printf("----------------------\n");
}

SubsecondTime
DramWriteQueuePerfModel::loadWriteLatency()
{
   const auto getLatencyValue = [](const String& key) -> std::optional<float> {
      if (Sim()->getCfg()->hasKey(key + "/write_latency")) return Sim()->getCfg()->getFloat(key + "/write_latency");
      if (Sim()->getCfg()->hasKey(key + "/latency")) return Sim()->getCfg()->getFloat(key + "/latency");
      return std::nullopt;
   };

   const auto latency = getLatencyValue("perf_model/" + DramCntlrInterface::getTechnology().second)
                              .value_or(*getLatencyValue("perf_model/dram"));

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<double>::NStoFS(latency));
}

SubsecondTime DramWriteQueuePerfModel::loadEnqueueLatency()
{
   const String key = "perf_model/dram/write_queue/enqueue_latency";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   LOG_ASSERT_ERROR(value >= 0 && value <= UINT32_MAX, "Invalid value for ['%s']", key.c_str());

   return SubsecondTime::NS(value);
}

UInt32 DramWriteQueuePerfModel::loadNumEntries()
{
   const String key = "perf_model/dram/write_queue/num_entries";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : UINT32_MAX;
   LOG_ASSERT_ERROR(value >= 0 && value <= UINT32_MAX, "Invalid value for ['%s']", key.c_str());

   return static_cast<UInt32>(value);
}

bool DramWriteQueuePerfModel::loadMerging()
{
   return Sim()->getCfg()->getBoolDefault("perf_model/dram/write_queue/merging", true);
}

UInt8 DramWriteQueuePerfModel::loadBurstSize()
{
   const String key = "perf_model/dram/write_queue/burst_size";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 1;
   LOG_ASSERT_ERROR(value >= 1 && value <= UINT8_MAX, "Invalid value for ['%s']", key.c_str());

   return static_cast<UInt8>(value);
}

std::unique_ptr<DramWriteQueuePerfModel>
DramWriteQueuePerfModel::create(const core_id_t core_id)
{
   if (Sim()->getCfg()->getBoolDefault("perf_model/dram/write_queue/enabled", false))
      return std::unique_ptr<DramWriteQueuePerfModel>(new DramWriteQueuePerfModel(core_id));

   return nullptr;
}
