#ifndef WRITE_BUFFER_CNTLR_H
#define WRITE_BUFFER_CNTLR_H

#include "write_buffer_entry.h"
#include "write_buffer_perf_model.h"
#include "write_buffer.h"
#include "subsecond_time.h"
#include "shmem_perf_model.h"
#include "simulator.h"
#include "config.hpp"

namespace ParametricDramDirectoryMSI
{

class CacheCntlrWrBuff;

class WriteBufferCntlr
{
public:

   WriteBufferCntlr(CacheCntlrWrBuff* cache_cntlr, const String& cache_name);
   WriteBufferCntlr(CacheCntlrWrBuff* cache_cntlr, UInt32 num_entries, const SubsecondTime& insertion_latency,
                    bool coalescing, bool asynchronous);

   virtual ~WriteBufferCntlr();

   SubsecondTime insert(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length,
                        ShmemPerfModel::Thread_t thread_num, UInt64 eid);
   SubsecondTime insert(const WriteBufferEntry& entry);

   SubsecondTime flush();
   SubsecondTime flushAll();

   bool isEmpty() { return isAsynchronous() ? m_buffer->isEmpty() : m_perf_model->isEmpty(); }
   bool isFull() { return isAsynchronous() ? m_buffer->isFull() : m_perf_model->isFull(); }

   [[nodiscard]] bool isAsynchronous() const { return m_buffer != nullptr; }

   void print(const String& desc = ""); // ONLY FOR DEBUG!

private:

   WriteBuffer* m_buffer;
   WriteBufferPerfModel* m_perf_model;
   CacheCntlrWrBuff* m_cache_cntlr;
   SubsecondTime m_insertion_latency;
   bool m_coalescing;

   void consume(const SubsecondTime& now);

   void send(const WriteBufferEntry& entry);

   static UInt32 getNumEntries(const String& cache_name);
   static SubsecondTime getInsertionLatency(const String& cache_name);
   static SubsecondTime getSendLatency(CacheCntlrWrBuff* cache_cntlr);
   static bool isCoalescing(const String& cache_name);
   static bool isAsynchronous(const String& cache_name);
};

}

#endif /* WRITE_BUFFER_CNTLR_H */