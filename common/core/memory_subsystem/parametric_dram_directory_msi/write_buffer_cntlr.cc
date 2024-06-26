#include "write_buffer_cntlr.h"
#include "cache_cntlr_wb.h"

namespace ParametricDramDirectoryMSI
{

WriteBufferCntlr::WriteBufferCntlr(CacheCntlrWrBuff* cache_cntlr, const String& cache_name) :
    WriteBufferCntlr(cache_cntlr,
                     getNumEntries(cache_name),
                     getInsertionLatency(cache_name),
                     isCoalescing(cache_name),
                     isAsynchronous(cache_name))
{ }

WriteBufferCntlr::WriteBufferCntlr(CacheCntlrWrBuff* cache_cntlr, UInt32 num_entries,
                                   const SubsecondTime& insertion_latency, bool coalescing, bool is_asynchronous) :
    m_buffer(nullptr),
    m_perf_model(nullptr),
    m_cache_cntlr(cache_cntlr),
    m_insertion_latency(insertion_latency),
    m_coalescing(coalescing)
{
   LOG_ASSERT_ERROR(num_entries > 0, "The <num_entries> of the write-buffer from %s must be greater than 0",
                    m_cache_cntlr->getCache()->getName().c_str());

   if (is_asynchronous)
   {
      m_buffer = m_coalescing ? (WriteBuffer*) new CoalescingWriteBuffer(num_entries) :
                                (WriteBuffer*) new NonCoalescingWriteBuffer(num_entries);
   }
   else
   {
      m_perf_model = new WriteBufferPerfModel(num_entries, coalescing, insertion_latency);
   }

//   printf("Cache %s | Async write-buffer: %s\n", m_cache_cntlr->getCache()->getName().c_str(), isAsynchronous() ? "true" : "false");
}

WriteBufferCntlr::~WriteBufferCntlr()
{
   delete m_buffer;
   delete m_perf_model;
}

SubsecondTime
WriteBufferCntlr::insert(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length,
                         ShmemPerfModel::Thread_t thread_num, UInt64 eid)
{
   return insert(WriteBufferEntry(address, offset, data_buf, data_length, thread_num, eid));
}

SubsecondTime
WriteBufferCntlr::insert(const WriteBufferEntry& entry)
{
   if (isAsynchronous())
   {
      SubsecondTime latency = m_insertion_latency;
      SubsecondTime t_now   = m_cache_cntlr->getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
      consume(t_now);

      if (m_coalescing && m_buffer->isPresent(entry.getAddress()))
         dynamic_cast<CoalescingWriteBuffer*>(m_buffer)->update(entry);
      else
      {
         SubsecondTime t_start      = m_buffer->isEmpty() ? t_now : m_buffer->getLastReleaseTime();
         SubsecondTime send_latency = getSendLatency(m_cache_cntlr);
         if (m_buffer->isFull())
            latency += flush();
         m_buffer->insert(entry, t_start + send_latency);

         // for debug
//         dynamic_cast<WriteBuffer *>(m_buffer)->print(m_cache_cntlr->getCache()->getName());
//         printf("Time: %lu | Written [%lx] in the writebuffer (%luns)\n", t_now.getNS(), entry.getAddress(), latency.getNS());
      }
      return latency;
   }

   send(entry);
   return m_perf_model->getInsertionLatency(entry.getAddress(), getSendLatency(m_cache_cntlr), m_cache_cntlr->getShmemPerfModel());
}

SubsecondTime
WriteBufferCntlr::flush()
{
   if (isAsynchronous())
   {
      SubsecondTime t_now   = m_cache_cntlr->getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD);
      SubsecondTime latency = m_buffer->getFirstReleaseTime() - t_now;
      send(m_buffer->remove());
      return latency;
   }

   return m_perf_model->flush(m_cache_cntlr->getShmemPerfModel());
}

SubsecondTime
WriteBufferCntlr::flushAll()
{
   if (isAsynchronous())
   {
      SubsecondTime latency = SubsecondTime::Zero();
      for (UInt32 i = 0; i < m_buffer->getSize(); i++)
         latency += flush();
      return latency;
   }

   return m_perf_model->flushAll(m_cache_cntlr->getShmemPerfModel());
}

void WriteBufferCntlr::consume(const SubsecondTime& now)
{
   while (!m_buffer->isEmpty() && now > m_buffer->getFirstReleaseTime())
      flush();
}

void
WriteBufferCntlr::send(const WriteBufferEntry& entry)
{
//   auto cache_cntlr = m_cache_cntlr->m_next_cache_cntlr;
//   cache_cntlr->writeCacheBlock(e.getAddress(), e.getOffset(), e.getDataBuf(), e.getDataLength(), e.getThreadNum(), e.getEpochID());

   m_cache_cntlr->sendByWriteBuffer(entry);
}

UInt32
WriteBufferCntlr::getNumEntries(const String& cache_name)
{
   const String key = "perf_model/" + cache_name + "/writebuffer/num_entries";
   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : UINT32_MAX;
}

SubsecondTime
WriteBufferCntlr::getInsertionLatency(const String& cache_name)
{
   const String key = "perf_model/" + cache_name + "/writebuffer/insertion_latency";
   return Sim()->getCfg()->hasKey(key) ? SubsecondTime::NS(Sim()->getCfg()->getInt(key)) : SubsecondTime::Zero();
}

SubsecondTime
WriteBufferCntlr::getSendLatency(CacheCntlrWrBuff* cache_cntlr)
{
   // FIXME: Check the latency according to technology and access type...
   if (cache_cntlr->isLastLevel())
      return cache_cntlr->getMemoryManager()->getCostNvm(DramCntlrInterface::WRITE);

   return cache_cntlr->getMemoryManager()->getCost(cache_cntlr->m_next_cache_cntlr->m_mem_component,
                                                   CachePerfModel::ACCESS_CACHE_DATA_AND_TAGS);
}

bool
WriteBufferCntlr::isCoalescing(const String& cache_name)
{
   const String default_key = "perf_model/writebuffer/coalescing";
   const String key = "perf_model/" + cache_name + "/writebuffer/coalescing";

   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getBool(key) : Sim()->getCfg()->getBoolDefault(default_key, false);
}

bool
WriteBufferCntlr::isAsynchronous(const String& cache_name)
{
   const String default_key = "perf_model/writebuffer/asynchronous";
   const String key = "perf_model/" + cache_name + "/writebuffer/asynchronous";

   return Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getBool(key) : Sim()->getCfg()->getBoolDefault(default_key, false);
}

/************************************************************
 * ONLY FOR DEBUG!
 ************************************************************/
void WriteBufferCntlr::print(const String& desc)
{
   if (isAsynchronous()) m_buffer->print(desc);
   else m_perf_model->print(desc);
}

}