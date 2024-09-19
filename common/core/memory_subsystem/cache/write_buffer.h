#ifndef WRITE_BUFFER_H
#define WRITE_BUFFER_H

#include "shmem_perf_model.h"
#include "subsecond_time.h"
#include "write_buffer_entry.h"

#include <algorithm>
#include <deque>
#include <map>
#include <variant>

class WriteBuffer
{
public:
   using WriteBufferEntryPair = std::pair<std::variant<WriteBufferEntry, IntPtr>, SubsecondTime>;

   explicit WriteBuffer(UInt32 num_entries) :
       m_num_entries(num_entries) {}
   virtual ~WriteBuffer() = default;

   virtual void insert(const WriteBufferEntry& entry, SubsecondTime time) = 0;
   virtual WriteBufferEntry remove()                                      = 0;
   // virtual WriteBufferEntry get(IntPtr address) = 0;
   virtual bool isPresent(IntPtr address) = 0;

   SubsecondTime getFirstReleaseTime() { return m_queue.front().second; }
   SubsecondTime getLastReleaseTime() { return m_queue.back().second; }

   [[nodiscard]] constexpr bool isEmpty() const noexcept { return m_queue.empty(); }
   [[nodiscard]] constexpr bool isFull() const noexcept { return m_queue.size() >= m_num_entries; }

   [[nodiscard]] constexpr UInt32 getSize() const noexcept { return m_queue.size(); }
   [[nodiscard]] constexpr UInt32 getCapacity() const noexcept { return m_num_entries; }

   // for debug
   void print(const String& desc = "");
   virtual std::tuple<IntPtr, SubsecondTime> getEntryInfo(WriteBufferEntryPair& entry) = 0;

protected:
   static const UInt32 DEFAULT_NUMBER_ENTRIES = 32;

   UInt32 m_num_entries;
   std::deque<WriteBufferEntryPair> m_queue;
};

class NonCoalescingWriteBuffer : public WriteBuffer
{
public:
   explicit NonCoalescingWriteBuffer(UInt32 num_entries = DEFAULT_NUMBER_ENTRIES);
   ~NonCoalescingWriteBuffer() override;

   void insert(const WriteBufferEntry& entry, SubsecondTime time) override;
   WriteBufferEntry remove() override;
   // WriteBufferEntry get(IntPtr address) override;
   bool isPresent(IntPtr address) override;

   std::tuple<IntPtr, SubsecondTime> getEntryInfo(WriteBufferEntryPair& e) override;
};

class CoalescingWriteBuffer : public WriteBuffer
{
public:
   explicit CoalescingWriteBuffer(UInt32 num_entries = DEFAULT_NUMBER_ENTRIES);
   ~CoalescingWriteBuffer() override;

   void insert(const WriteBufferEntry& entry, SubsecondTime time) override;
   void update(const WriteBufferEntry& entry);
   WriteBufferEntry remove() override;
   // virtual WriteBufferEntry get(IntPtr address);
   bool isPresent(IntPtr address) override;

   std::tuple<IntPtr, SubsecondTime> getEntryInfo(WriteBufferEntryPair& entry) override;

private:
   std::map<IntPtr, WriteBufferEntry> m_map;
};

#endif /* WRITE_BUFFER_H */