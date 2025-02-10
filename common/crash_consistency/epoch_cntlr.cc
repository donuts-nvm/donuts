#include "epoch_cntlr.h"
#include "hooks_manager.h"
#include "simulator.h"

#include <algorithm>
#include <core_manager.h>

EpochCntlr::EpochCntlr(EpochManager& epoch_manager,
                       const UInt32 vd_id,
                       const SubsecondTime& max_interval_time,
                       const UInt64 max_interval_instr,
                       const std::vector<core_id_t>& cores) :
    m_epoch_manager(epoch_manager),
    m_vd(vd_id),
    m_cores(cores),
    m_watchdog(&EpochCntlr::notifyEpochEnding, max_interval_time, max_interval_instr, cores)
{
   const auto _start = [](auto, const UInt64 eid) -> SInt64
   {
      printf("Starting an epoch: [%lu]\n", eid);
      return 0;
   };
   Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_START, _start, 0);

   const auto _end = [](auto, const UInt64 eid) -> SInt64
   {
      printf("Ending an epoch: [%lu]\n", eid);
      return 0;
   };
   Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_END, _end, 0);

   const auto _persisted = [](auto, const UInt64 eid) -> SInt64
   {
      printf("Epoch persisted: [%lu]\n", eid);
      return 0;
   };
   Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_PERSISTED, _persisted, 0);
}

EpochCntlr::~EpochCntlr() = default;

void
EpochCntlr::newEpoch()
{
   /*
    * Get infos:
    * - IntPtr getFirstInstruction();
    * - SubsecondTime getTimeStart(); ???
    * - UInt64 getTotalInstructionsStart(); ???
    * - UInt64 getTotalVDInstructionsStart(); ???
    */

   const auto core_manager = Sim()->getCoreManager();
   const auto core         = m_vd.getEpochID() == 0 ? core_manager->getCoreFromID(m_cores[0]) : core_manager->getCurrentCore();
   // printf("[ New Epoch ] PCs: [%lx %lx]\n", core->getProgramCounter(), core->getLastPCToDCache());

   printf("[ New Epoch ]\n"
          "- PC: %lx %lx\n"
          "- Time: %lu\n"
          "- Instruction: %lu %lu\n",
          core->getProgramCounter(), core->getLastPCToDCache(),
          Sim()->getClockSkewMinimizationServer()->getGlobalTime().getNS(),
          Sim()->getCoreManager()->getInstructionCount(), Sim()->getCoreManager()->getInstructionCount(m_cores));

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_START, m_vd.increment(), false);
}

void
EpochCntlr::commit(const CheckpointReason reason)
{
   /*
    * Get infos:
    * - UInt64 getNumInstructions();
    * - UInt64 getNumWrites();
    * - UInt64 getNumLogs();
    * - SubsecondTime getDuration();
    * - UInt32 getCheckpointSize();
    * - SubsecondTime getEstimatedPersistenceTime();
    */

   printf("[ Ending an Epoch ]\n"
          "- Number of Instructions: %lu\n"
          "- Number of Writes: %lu\n"
          "- Number of Logs: %lu\n"
          "- Duration: %lu ns\n"
          "- Checkpoint Size: %lu B\n"
          "- Persistence Time: %lu ns\n",
          0L, 0L, 0L, SubsecondTime::Zero().getNS(), 0L, SubsecondTime::Zero().getNS());

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_END, m_vd.getEpochID(), false);
   m_watchdog.refresh();
   newEpoch();
}

void
EpochCntlr::notifyEpochEnding(const WatchdogEvent event, const UInt64 arg)
{
   if (event == WatchdogEvent::TIMEOUT)
      Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT, arg, false);
   else
      Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT_INS, static_cast<subsecond_time_t>(arg).m_time, false);
}

std::vector<core_id_t>
EpochCntlr::generateCoresArray(core_id_t const core_start, const core_id_t core_end)
{
   LOG_ASSERT_ERROR(core_start <= core_end, "The core_start must be equals or less than core_end [core_start = %u, core_end = %u]",
                    core_start, core_end);
   std::vector<core_id_t> cores(core_end - core_start + 1);
   std::ranges::iota(cores, core_start);

   return cores;
}
