#include "write_buffer_perf_model.h"
#include "stats.h"

WriteBufferPerfModel::WriteBufferPerfModel(const String& cache_name, const core_id_t core_id,
                                           const UInt32 num_entries,
                                           const bool coalescing,
                                           const SubsecondTime& insertion_latency) :
    m_num_entries(num_entries),
    m_coalescing(coalescing),
    m_insertion_latency(insertion_latency),
    m_total_latency(SubsecondTime::Zero()),
    m_num_accesses(0),
    m_num_insertions(0),
    m_num_overflows(0)
{
   const String write_buffer_name = cache_name + "-write-buffer";

   registerStatsMetric(write_buffer_name, core_id, "total-accesses", &m_num_accesses);
   registerStatsMetric(write_buffer_name, core_id, "total-insertions", &m_num_insertions);
   registerStatsMetric(write_buffer_name, core_id, "total-overflows", &m_num_overflows);
   registerStatsMetric(write_buffer_name, core_id, "total-access-latency", &m_total_latency);
   // registerStatsMetric(write_buffer_name, core_id, "total-queueing-delay", &m_total_queueing_delay);
}

void WriteBufferPerfModel::consume(const SubsecondTime& t_now)
{
   while (!m_queue.empty() && t_now >= m_queue.front().second)
      m_queue.pop_front();
}

SubsecondTime
WriteBufferPerfModel::getEnqueueLatency(IntPtr address, const SubsecondTime& send_latency, const SubsecondTime& t_now)
{
   SubsecondTime latency     = m_insertion_latency;
   consume(t_now);

   auto same_address = [&](auto& e) {
      return e.first == address;
   };

   if (!m_coalescing || std::ranges::find_if(m_queue.rbegin(), m_queue.rend(), same_address) == m_queue.rend())
   {
      const SubsecondTime t_start = m_queue.empty() ? t_now : m_queue.back().second;
      if (m_queue.size() >= m_num_entries)
      {
         latency += m_queue.front().second - t_now;
         m_queue.pop_front();
         m_num_overflows++;
      }
      m_queue.emplace_back(address, t_start + send_latency);
      m_num_insertions++;

      // print();
   }
   m_num_accesses++;
   return latency;
}

SubsecondTime
WriteBufferPerfModel::flush(const SubsecondTime& t_now) const
{
   return m_queue.front().second - t_now;
}

SubsecondTime
WriteBufferPerfModel::flushAll(const SubsecondTime& t_now) const
{
   SubsecondTime latency = SubsecondTime::Zero();
   for (UInt32 i = 0; i < m_queue.size(); i++)
      latency += flush(t_now);
   return latency;
}

/************************************************************
 * ONLY FOR DEBUG!
 ************************************************************/
void WriteBufferPerfModel::print(const String& desc) const
{
   UInt32 index = 0;
   if (!desc.empty())
   {
      printf("*** %s ***\n", desc.c_str());
   }

   printf("----------------------\n");
   for (const auto& [addr, time]: m_queue) printf("%4u [%12lX] -> (%lu)\n", index++, addr, time.getNS());
   printf("----------------------\n");
}
