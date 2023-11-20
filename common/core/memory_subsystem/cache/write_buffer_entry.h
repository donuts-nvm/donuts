#ifndef WRITE_BUFFER_ENTRY_H
#define WRITE_BUFFER_ENTRY_H

#include "shmem_perf_model.h"

class WriteBufferEntry {
public:

   /**
    * Creates a WriteBufferEntry.
    *
    * @param address
    * @param offset
    * @param data_buf
    * @param data_length
    * @param thread_num
    */
   WriteBufferEntry(IntPtr address, UInt32 offset, Byte* data_buf, UInt32 data_length,
                    ShmemPerfModel::Thread_t thread_num) :
       address(address),
       offset(offset),
       data_buf(data_buf),
       data_length(data_length),
       thread_num(thread_num)
   { }

   /**
    * Creates a WriteBufferEntry.
    *
    * @param orig
    */
   WriteBufferEntry(const WriteBufferEntry &orig) = default;

   /**
    * Destroys this WriteBufferEntry.
    */
   virtual ~WriteBufferEntry() = default;

   IntPtr getAddress() const { return address; }
   UInt32 getOffset() const { return offset; }
   Byte* getDataBuffer() const { return data_buf; };
   UInt32 getDataLength() const { return data_length; }
   ShmemPerfModel::Thread_t getThreadNum() const { return thread_num; }

private:

   IntPtr address;
   UInt32 offset;
   Byte* data_buf;
   UInt32 data_length;
   ShmemPerfModel::Thread_t thread_num;
};

#endif /* WRITE_BUFFER_ENTRY_H */