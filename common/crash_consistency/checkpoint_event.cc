#include "checkpoint_event.h"
#include "simulator.h"
#include "core_manager.h"
#include "performance_model.h"
#include "magic_server.h"

IntPtr
CheckpointEvent::getFirstInstruction() const
{
   // informado no início da época
   return Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache();
}

IntPtr
CheckpointEvent::getLastInstruction() const
{
   // informado no final da época... newEpoch()?
   return Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache();
}

CheckpointReason
CheckpointEvent::getReason() const
{
   return CheckpointReason::CACHE_THRESHOLD;
}

SubsecondTime
CheckpointEvent::getTime() const
{
   return Sim()->getClockSkewMinimizationServer()->getGlobalTime();
}

UInt64
CheckpointEvent::getTotalInstructions() const
{
   return MagicServer::getGlobalInstructionCount();
}

UInt64
CheckpointEvent::getTotalVDInstructions() const
{
   std::vector<core_id_t> cores = {0, 1, 2, 3};
   UInt64 count = 0;
   for(core_id_t core_id : cores)
      count += Sim()->getCoreManager()->getCoreFromID(core_id)->getInstructionCount();
   return count;
}

UInt64
CheckpointEvent::getNumInstructions() const
{
   // current_vd - last_vd
   return 0;
}

UInt64
CheckpointEvent::getNumLogs() const
{
   // (current_vd - last_vd) ou informado no final da época??
   return 0;
}

UInt64
CheckpointEvent::getNumWrites() const
{
   // (current_vd - last_vd) ou informado no final da época??
   return 0;
}

SubsecondTime
CheckpointEvent::getDuration() const
{
   // current_vd - last_vd
   return SubsecondTime::Zero();
}

UInt32
CheckpointEvent::getCheckpointSize() const
{
   UInt32 log_size = 64;
   return getNumLogs() * log_size;
}

SubsecondTime
CheckpointEvent::getEstimatedPersistenceTime() const
{
   return SubsecondTime::Zero();
}

void
CheckpointEvent::print() const
{
   printf("--------------------------------------------------\n"
          "- First Instruction %lu\n"
          "- Last Instruction: %lu\n"
          "- Reason: %s\n"
          "- Time: %lu ns\n"
          "- Total instructions: %lu\n"
          "- Total VD instructions: %lu\n"
          "- Number of instructions: %lu\n"
          "- Number of writes: %lu\n"
          "- Number of logs: %lu\n"
          "- Duration: %lu ns\n"
          "- Checkpoint size: %u B\n"
          "- Estimated persistence time: %lu ns\n"
          "--------------------------------------------------\n",
          getFirstInstruction(), getLastInstruction(), reasonToString(getReason()),
          getTime().getNS(), getTotalInstructions(), getTotalVDInstructions(),
          getNumInstructions(), getNumWrites(), getNumLogs(), getDuration().getNS(),
          getCheckpointSize(), getEstimatedPersistenceTime().getNS());
}
