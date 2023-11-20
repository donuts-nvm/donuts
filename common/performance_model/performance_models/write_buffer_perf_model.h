#ifndef WRITE_BUFFER_PERF_MODEL_H
#define WRITE_BUFFER_PERF_MODEL_H

#include "subsecond_time.h"
#include "shmem_perf_model.h"

#include <deque>
#include <algorithm>

class WriteBufferPerfModel
{
   public:

   explicit WriteBufferPerfModel(UInt32 num_entries, bool coalescing = true,
                        const SubsecondTime& insertion_latency = SubsecondTime::Zero());
   virtual ~WriteBufferPerfModel();

   SubsecondTime getInsertionLatency(IntPtr address, const SubsecondTime& send_latency, ShmemPerfModel *perf);

   SubsecondTime flush(ShmemPerfModel *perf);
   SubsecondTime flushAll(ShmemPerfModel *perf);

   // Statistics
   SubsecondTime getAvgLatency() { return m_total_latency / m_num_insertions; }
   [[nodiscard]] UInt64 getTotalAccesses() const { return m_num_accesses; }
   [[nodiscard]] UInt64 getTotalInsertions() const { return m_num_insertions; }
   [[nodiscard]] UInt64 getTotalOverflows() const { return m_num_overflows; }

   private:

   UInt32 m_num_entries;
   bool m_coalescing;
   SubsecondTime m_insertion_latency;
   std::deque<std::pair<IntPtr, SubsecondTime>> m_queue;

   // Statistics
   SubsecondTime m_total_latency;
   UInt64 m_num_accesses;
   UInt64 m_num_insertions;
   UInt64 m_num_overflows;

   void consume(const SubsecondTime& t_now);
};

#endif /* WRITE_BUFFER_PERF_MODEL_H */