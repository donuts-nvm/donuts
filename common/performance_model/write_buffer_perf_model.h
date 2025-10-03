#pragma once

#include "subsecond_time.h"

#include <deque>

class WriteBufferPerfModel final
{
public:
   explicit WriteBufferPerfModel(const String& cache_name, core_id_t core_id,
                                 UInt32 num_entries,
                                 bool coalescing                        = true,
                                 const SubsecondTime& insertion_latency = SubsecondTime::Zero());

   SubsecondTime getEnqueueLatency(IntPtr address, const SubsecondTime& send_latency, const SubsecondTime& t_now);

   [[nodiscard]] SubsecondTime flush(const SubsecondTime& t_now) const;
   [[nodiscard]] SubsecondTime flushAll(const SubsecondTime& t_now) const;

   [[nodiscard]] bool isEmpty() const { return m_queue.empty(); }
   [[nodiscard]] bool isFull() const { return m_queue.size() >= m_num_entries; }

   // Statistics
   [[nodiscard]] SubsecondTime getAvgLatency() const { return m_total_latency / m_num_insertions; }
   [[nodiscard]] UInt64 getTotalAccesses() const { return m_num_accesses; }
   [[nodiscard]] UInt64 getTotalInsertions() const { return m_num_insertions; }
   [[nodiscard]] UInt64 getTotalOverflows() const { return m_num_overflows; }

   // For Debug
   void print(const String& desc = "") const;

private:
   UInt32 m_num_entries;
   bool m_coalescing;
   SubsecondTime m_insertion_latency;
   std::deque<std::pair<IntPtr, SubsecondTime>> m_queue;

   // Statistics
   SubsecondTime m_total_latency;
   SubsecondTime m_total_queueing_delay;
   UInt64 m_num_accesses;
   UInt64 m_num_insertions;
   UInt64 m_num_overflows;

   void consume(const SubsecondTime& t_now);
};
