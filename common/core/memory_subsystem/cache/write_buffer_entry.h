#ifndef WRITE_BUFFER_ENTRY_H
#define WRITE_BUFFER_ENTRY_H

#include "shmem_perf_model.h"

class WriteBufferEntry
{
public:

   /**
    * Creates a WriteBufferEntry.
    *
    * @param address
    * @param offset
    * @param data_buf
    * @param data_length
    * @param thread_num
    * @param eid
    */
   explicit WriteBufferEntry(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length,
                             ShmemPerfModel::Thread_t thread_num, UInt64 eid) noexcept :
       address(address),
       offset(offset),
       data_buf(data_buf),
       data_length(data_length),
       thread_num(thread_num),
       eid(eid) {}

   /**
    * Creates a WriteBufferEntry.
    *
    * @param orig
    */
   WriteBufferEntry(const WriteBufferEntry& orig) noexcept            = default;
   WriteBufferEntry(WriteBufferEntry&& other) noexcept                = default;
   WriteBufferEntry& operator=(const WriteBufferEntry& orig) noexcept = default;
   WriteBufferEntry& operator=(WriteBufferEntry&& other) noexcept     = default;

   /**
    * Destroys this WriteBufferEntry.
    */
   virtual ~WriteBufferEntry() = default;

   [[nodiscard]] IntPtr getAddress() const { return address; }
   [[nodiscard]] UInt32 getOffset() const { return offset; }
   [[nodiscard]] Byte* getDataBuf() const { return data_buf; };
   [[nodiscard]] UInt32 getDataLength() const { return data_length; }
   [[nodiscard]] ShmemPerfModel::Thread_t getThreadNum() const { return thread_num; }
   [[nodiscard]] UInt64 getEpochID() const { return eid; }

private:
   IntPtr address;
   UInt32 offset;
   Byte* data_buf;
   UInt32 data_length;
   ShmemPerfModel::Thread_t thread_num;
   UInt64 eid;
};

#endif /* WRITE_BUFFER_ENTRY_H */