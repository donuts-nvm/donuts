#include "write_buffer.h"

NonCoalescingWriteBuffer::NonCoalescingWriteBuffer(const UInt32 num_entries) :
    WriteBuffer(num_entries) {}

NonCoalescingWriteBuffer::~NonCoalescingWriteBuffer() = default;

void NonCoalescingWriteBuffer::insert(const WriteBufferEntry& entry, const SubsecondTime& time)
{
   m_queue.emplace_back(entry, time);
}

WriteBufferEntry NonCoalescingWriteBuffer::remove()
{
   const auto entry = std::get<WriteBufferEntry>(m_queue.front().first);
   m_queue.pop_front();

   return entry;
}

bool NonCoalescingWriteBuffer::isPresent(const IntPtr address)
{
   const auto same_address = [&](auto& e) {
      return std::get<WriteBufferEntry>(e.first).address == address;
   };
   return std::ranges::find_if(m_queue.begin(), m_queue.end(), same_address) != m_queue.end();
}

std::pair<IntPtr, SubsecondTime> NonCoalescingWriteBuffer::getEntryInfo(const WriteBufferEntryPair& e)
{
   const auto entry = std::get<WriteBufferEntry>(e.first);
   return std::make_pair(entry.address, e.second);
}

CoalescingWriteBuffer::CoalescingWriteBuffer(const UInt32 num_entries) :
    WriteBuffer(num_entries) {}

CoalescingWriteBuffer::~CoalescingWriteBuffer() = default;

void CoalescingWriteBuffer::insert(const WriteBufferEntry& entry, const SubsecondTime& time)
{
   m_queue.emplace_back(entry.address, time);
   m_map.insert(std::make_pair(entry.address, entry));
}

void CoalescingWriteBuffer::update(const WriteBufferEntry& entry)
{
   m_map.find(entry.address)->second = entry;
}

WriteBufferEntry CoalescingWriteBuffer::remove()
{
   const auto entry = m_map.at(std::get<IntPtr>(m_queue.front().first));
   m_map.erase(entry.address);
   m_queue.pop_front();

   return entry;
}

bool CoalescingWriteBuffer::isPresent(const IntPtr address)
{
   return m_map.contains(address);
}

std::pair<IntPtr, SubsecondTime> CoalescingWriteBuffer::getEntryInfo(const WriteBufferEntryPair& e)
{
   const auto entry = m_map.at(std::get<IntPtr>(e.first));
   return std::make_pair(entry.address, e.second);
}

/************************************************************
 * ONLY FOR DEBUG!
 ************************************************************/
void WriteBuffer::print(const String& desc)
{
   UInt32 index = 0;
   if (!desc.empty())
   {
      printf("*** %s ***\n", desc.c_str());
   }

   printf("----------------------\n");
   for (auto& entry: m_queue)
   {
      const auto [addr, time] = getEntryInfo(entry);
      printf("%4u [%12lx] -> (%lu)\n", index++, addr, time.getNS());
   }
   printf("----------------------\n");
}
