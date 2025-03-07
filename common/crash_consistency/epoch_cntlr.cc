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
   // const auto _start = [](auto, const UInt64 eid) -> SInt64
   // {
   //    printf("Starting an epoch: [%lu]\n", eid);
   //    return 0;
   // };
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_START, _start, 0);
   //
   // const auto _end = [](auto, const UInt64 eid) -> SInt64
   // {
   //    printf("Ending an epoch: [%lu]\n", eid);
   //    return 0;
   // };
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_END, _end, 0);
   //
   // const auto _persisted = [](auto self, const UInt64 eid) -> SInt64
   // {
   //    printf("Epoch persisted: [%lu]\n", eid);
   //    return 0;
   // };
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_PERSISTED, _persisted, reinterpret_cast<UInt64>(this));
}

EpochCntlr::~EpochCntlr() = default;

void
EpochCntlr::newEpoch()
{
   const auto core       = m_vd.getEpochID() == 0 ? Sim()->getCoreManager()->getCoreFromID(m_cores[0]) :
                                                    Sim()->getCoreManager()->getCurrentCore();
   const auto epoch_info = EpochInfo::start(m_vd.increment(), core->getProgramCounter(), SystemSnapshot::capture(m_cores));
   m_epochs.emplace(epoch_info.getEpochID(), epoch_info);

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_START, epoch_info.getEpochID(), false);
}

void
EpochCntlr::commit(const CheckpointReason reason)
{
   m_epochs.at(m_vd.getEpochID()).finalize(reason, SystemSnapshot::capture(m_cores));

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
