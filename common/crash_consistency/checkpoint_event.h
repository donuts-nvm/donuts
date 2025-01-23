#pragma once

#include "log.h"
#include "subsecond_time.h"

class CheckpointInfo
{
public:
   enum class Reason
   {
      PERIODIC_TIME,
      PERIODIC_INSTRUCTIONS,
      CACHE_SET_THRESHOLD,
      CACHE_THRESHOLD
   };

   static const char* reasonToString(const Reason reason)
   {
      switch (reason)
      {
         case Reason::PERIODIC_TIME:
            return "Periodic Time";
         case Reason::PERIODIC_INSTRUCTIONS:
            return "Periodic Instructions";
         case Reason::CACHE_SET_THRESHOLD:
            return "Cache Set Threshold";
         case Reason::CACHE_THRESHOLD:
            return "Cache Threshold";
      }
      LOG_PRINT_ERROR("Unknown reason");
   }

   virtual ~CheckpointInfo() = default;

   /**
    * @return address of the epoch's first instruction in the checkpoint
    */
   [[nodiscard]] virtual IntPtr getFirstInstruction() const = 0;

   /**
    * @return address of the epoch's last instruction in the checkpoint
    */
   [[nodiscard]] virtual IntPtr getLastInstruction() const = 0;

   /**
    * @return address of the epoch's first and last instruction in the checkpoint
    */
   [[nodiscard]] std::pair<IntPtr, IntPtr> getFirstAndLastInstruction() const
   {
      return std::make_pair(getFirstInstruction(), getLastInstruction());
   }

   /**
    * @return the checkpoint reason
    */
   [[nodiscard]] virtual Reason getReason() const = 0;

   /**
    * @return the checkpoint time
    */
   [[nodiscard]] virtual SubsecondTime getTime() const = 0;

   /**
    * @return the total number of instructions
    */
   [[nodiscard]] virtual UInt64 getTotalInstructions() const = 0;

   /**
    * @return the total number of instructions in the versioned domain
    */
   [[nodiscard]] virtual UInt64 getTotalVDInstructions() const = 0;

   /**
    * @return number of instructions executed in the checkpoint
    */
   [[nodiscard]] virtual UInt64 getNumInstructions() const = 0;

   /**
    * @return number of writes in the current epoch
    */
   [[nodiscard]] virtual UInt64 getNumWrites() const = 0;

   /**
    * @return number of performed logs in the checkpoint
    */
   [[nodiscard]] virtual UInt64 getNumLogs() const = 0;

   /**
    * @return the write amplification rate
    */
   [[nodiscard]] virtual float getWriteAmplification() const
   {
      return getNumLogs() / getNumWrites();
   }

   /**
    * @return the epoch length of this checkpoint
    */
   [[nodiscard]] virtual SubsecondTime getDuration() const = 0;

   /**
    * @return the number of bytes of this checkpoint
    */
   [[nodiscard]] virtual UInt32 getCheckpointSize() const = 0;

   /**
    * @return the estimated persistence time
    */
   [[nodiscard]] virtual SubsecondTime getEstimatedPersistenceTime() const = 0;
};

using CheckpointReason = CheckpointInfo::Reason;

class CheckpointEvent final : CheckpointInfo
{
public:
   explicit CheckpointEvent(const CheckpointReason reason) :
       m_reason(reason) {}

   [[nodiscard]] IntPtr getFirstInstruction() const override;
   [[nodiscard]] IntPtr getLastInstruction() const override;
   [[nodiscard]] CheckpointReason getReason() const override;
   [[nodiscard]] SubsecondTime getTime() const override;
   [[nodiscard]] UInt64 getTotalInstructions() const override;
   [[nodiscard]] UInt64 getTotalVDInstructions() const override;
   [[nodiscard]] UInt64 getNumInstructions() const override;
   [[nodiscard]] UInt64 getNumWrites() const override;
   [[nodiscard]] UInt64 getNumLogs() const override;
   [[nodiscard]] SubsecondTime getDuration() const override;
   [[nodiscard]] UInt32 getCheckpointSize() const override;
   [[nodiscard]] SubsecondTime getEstimatedPersistenceTime() const override;

   void print() const;

private:
   CheckpointReason m_reason;
};
