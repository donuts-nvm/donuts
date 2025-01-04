#include "epoch_info.h"
#include "core_manager.h"
#include "simulator.h"

SystemSnapshot
SystemSnapshot::capture(const std::vector<core_id_t>& vd_cores)
{
   const auto now            = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
   const auto total_instr    = Sim()->getCoreManager()->getInstructionCount();
   const auto total_instr_vd = Sim()->getCoreManager()->getInstructionCount(vd_cores);

   return { now, total_instr, total_instr_vd };
}

EpochInfo::EpochInfo(const UInt64 eid, const IntPtr pc, const SystemSnapshot& snapshot) :
    m_eid(eid),
    m_pc(pc),
    m_snapshot(snapshot),
    m_reason(),
    m_duration(SubsecondTime::Zero()),
    m_num_instr(0),
    m_persistence_time(SubsecondTime::Zero()) {}

EpochInfo
EpochInfo::start(const UInt64 eid, const IntPtr pc, const SystemSnapshot& snapshot)
{
   return EpochInfo(eid, pc, snapshot);
}

void
EpochInfo::finalize(const CheckpointReason reason, const SystemSnapshot& snapshot)
{
   m_reason    = reason;
   m_duration  = snapshot.getGlobalTime() - m_snapshot.getGlobalTime();
   m_num_instr = snapshot.getTotalInstructionsVD() - m_snapshot.getTotalInstructionsVD();
}

void
EpochInfo::persistedIn(const SubsecondTime& time)
{
   m_persistence_time = time;
}

void
EpochInfo::print() const
{
   printf("============================================================\n"
          " Epoch ID: %lu\n"
          "------------------------------------------------------------\n"
          " Initial Snapshot\n"
          " - Program Counter: %lx\n"
          " - Global Time: %lu ns\n"
          " - Total instructions: %lu\n"
          " - Total instructions in VD: %lu\n"
          " Checkpoint\n"
          " - Reason: %i\n"
          " - Size: %lu B\n"
          " General Statistics\n"
          " - Duration: %lu ns\n"
          " - Number of instructions: %lu\n"
          " NVM Statistics\n"
          " - Reads: %lu\n"
          " - Writes: %lu\n"
          " - Logs: %lu\n"
          " - Write Amplification: %.2f\n"
          " - Persistence time: %lu ns\n"
          "============================================================\n",
          getEpochID(),
          getProgramCounter(), getGlobalTime().getNS(), getTotalInstructions(), getTotalInstructionsVD(),
          static_cast<int>(getReason()), getCheckpointSize(),
          getDuration().getNS(), getNumInstructions(),
          getNumReads(), getNumWrites(), getNumLogs(), getWriteAmplification().value_or(0), getPersistenceTime().getNS());
}
