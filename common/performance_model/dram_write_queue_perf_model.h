#pragma once

#include "subsecond_time.h"

#include <deque>
#include <memory>

class DramWriteQueuePerfModel
{
public:
   explicit DramWriteQueuePerfModel(core_id_t core_id, UInt32 num_entries, bool merging, UInt8 burst_size,
                                    const SubsecondTime& write_latency, SubsecondTime enqueue_latency);

   SubsecondTime sendAndCalculateDelay(IntPtr address, const SubsecondTime& write_msg_time);

   static std::unique_ptr<DramWriteQueuePerfModel> create(core_id_t core_id);

private:
   void consume(const SubsecondTime& now);
   void print() const;

   static SubsecondTime loadWriteLatency();
   static SubsecondTime loadEnqueueLatency();
   static UInt32 loadNumEntries();
   static bool loadMerging();
   static UInt8 loadBurstSize();

   SubsecondTime m_time;
   SubsecondTime m_write_cost;
   SubsecondTime m_enqueue_cost;
   UInt32 m_num_entries;
   bool m_merging;
   UInt8 m_burst_size;

   std::deque<std::pair<IntPtr, SubsecondTime>> m_queue;

   // Statistics
   SubsecondTime m_total_latency;
   SubsecondTime m_total_queueing_delay;
   UInt64 m_num_accesses;
   UInt64 m_num_insertions;
   UInt64 m_num_overflows;
};
