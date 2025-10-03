#pragma once

#include <algorithm>
#include <deque>
#include <map>
#include <variant>

#include "shmem_perf_model.h"
#include "subsecond_time.h"

class WriteBuffer
{
public:
   struct Entry
   {
      IntPtr address;
      UInt32 offset;
      Byte* data_buf;
      UInt32 data_length;
      ShmemPerfModel::Thread_t thread_num;
      UInt64 eid = 0;
   };

   using WriteBufferEntryPair = std::pair<std::variant<Entry, IntPtr>, SubsecondTime>;

   explicit WriteBuffer(const UInt32 num_entries) :
       m_num_entries(num_entries) {}

   virtual ~WriteBuffer() = default;

   virtual void insert(const Entry& entry, const SubsecondTime& time) = 0;
   void insert(const SubsecondTime& time,
               const IntPtr address, const UInt32 offset,
               Byte* data_buf, const UInt32 data_length,
               const ShmemPerfModel::Thread_t thread_num,
               const UInt64 eid = 0)
   {
      insert({ address, offset, data_buf, data_length, thread_num, eid }, time);
   }

   virtual Entry remove() = 0;

   // virtual Entry get(IntPtr address) = 0;

   virtual bool isPresent(IntPtr address) = 0;

   [[nodiscard]] constexpr bool isEmpty() const noexcept { return m_queue.empty(); }
   [[nodiscard]] constexpr bool isFull() const noexcept { return m_queue.size() >= m_num_entries; }

   [[nodiscard]] constexpr UInt32 getSize() const noexcept { return m_queue.size(); }
   [[nodiscard]] constexpr UInt32 getCapacity() const noexcept { return m_num_entries; }

protected:
   static constexpr UInt32 DEFAULT_NUMBER_ENTRIES = 32;

   UInt32 m_num_entries;
   std::deque<WriteBufferEntryPair> m_queue;

   SubsecondTime getFirstReleaseTime() { return m_queue.front().second; }
   SubsecondTime getLastReleaseTime() { return m_queue.back().second; }

   // For Debug
   void print(const String& desc = "");
   virtual std::pair<IntPtr, SubsecondTime> getEntryInfo(const WriteBufferEntryPair& entry) = 0;
};

using WriteBufferEntry = WriteBuffer::Entry;

class NonCoalescingWriteBuffer final : public WriteBuffer
{
public:
   explicit NonCoalescingWriteBuffer(UInt32 num_entries = DEFAULT_NUMBER_ENTRIES);
   ~NonCoalescingWriteBuffer() override;

   void insert(const WriteBufferEntry& entry, const SubsecondTime& time) override;
   WriteBufferEntry remove() override;
   // WriteBufferEntry get(IntPtr address) override;
   bool isPresent(IntPtr address) override;

   std::pair<IntPtr, SubsecondTime> getEntryInfo(const WriteBufferEntryPair& e) override;
};

class CoalescingWriteBuffer final : public WriteBuffer
{
public:
   explicit CoalescingWriteBuffer(UInt32 num_entries = DEFAULT_NUMBER_ENTRIES);
   ~CoalescingWriteBuffer() override;

   void insert(const WriteBufferEntry& entry, const SubsecondTime& time) override;
   void update(const WriteBufferEntry& entry);
   WriteBufferEntry remove() override;
   // virtual WriteBufferEntry get(IntPtr address);
   bool isPresent(IntPtr address) override;

   std::pair<IntPtr, SubsecondTime> getEntryInfo(const WriteBufferEntryPair& e) override;

private:
   std::map<IntPtr, WriteBufferEntry> m_map;
};
