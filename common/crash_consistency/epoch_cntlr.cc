#include "epoch_cntlr.h"
#include "hooks_manager.h"
#include "simulator.h"
#include "system_snapshot.h"

VersionedDomain::VersionedDomain(const UInt32 id, const core_id_t first_core, const core_id_t last_core) :
    m_id(id), m_eid(0)
{
   for (core_id_t core_id = first_core; core_id <= last_core; ++core_id)
   {
      registerStatsMetric("epoch", core_id, "id", &m_eid);
   }
}

EpochCntlr::EpochCntlr(EpochManager& epoch_manager,
                       const UInt32 vd_id,
                       const core_id_t first_core, const core_id_t last_core,
                       const SubsecondTime& max_interval_time,
                       const UInt64 max_interval_instr,
                       const std::vector<core_id_t>& cores) :
    m_epoch_manager(epoch_manager),
    m_vd(vd_id, first_core, last_core),
    m_cores(cores),
    m_watchdog(&EpochCntlr::notifyEpochEnding, max_interval_time, max_interval_instr, cores)
{
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_START, [](auto eid) { printf("Starting epoch: [%lu]\n", eid); });
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_END, [](auto eid) { printf("Ending epoch: [%lu]\n", eid); });
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_PERSISTED, [](auto eid) { printf("Epoch persisted: [%lu]\n", eid); });
}

void EpochCntlr::newEpoch()
{
   if (m_vd.getEpochID() == 0)
   {
      m_snapshots.emplace_back(SystemSnapshot::INITIAL());
   }

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_START, m_vd.increment(), false);
}

void EpochCntlr::commit(const CheckpointReason reason)
{
   const auto initial_snapshot = m_snapshots.back();
   const auto final_snapshot   = SystemSnapshot::capture(m_cores);

   GenericSnapshot<stats::MemInfo, stats::MemInfo> snapshot2(stats::getMemInfo(), stats::getGlobalTime());

   // auto pc = final_snapshot.getCoresInfo().getCycles(0);
   // auto d = final_snapshot - initial_snapshot;
   // // auto e = d.getCycles(final_snapshot.getCycles());
   // auto f = d.getCoresInfo().getMasterCoreCycles();

   printf("INITIAL SNAP:\n%s\n", initial_snapshot.toString().c_str());
   // printf("INITIAL SNAP:\n%s\n", initial_snapshot.asJson().c_str());
   printf("FINAL SNAP:\n%s\n", final_snapshot.toString().c_str());
   // printf("FINAL SNAP:\n%s\n", final_snapshot.asJson().c_str());

   // GenericSnapshot<stats::MemInfo, stats::MemInfo> snapshot3(stats::getMemInfo(), stats::getGlobalTime());
   // auto r = snapshot3 - snapshot2;
   // printf("ACESSOS NA DRAAAAMMMMM: (%lu - %lu) = %lu\n", snapshot3->dram.accesses, snapshot2->dram.accesses, r.getInfo().dram.accesses);

   auto delta = final_snapshot - initial_snapshot;
   printf("INTERVAL SNAP:\n%s\n", delta.toString().c_str());
   // printf("INTERVAL SNAP:\n%s\n", delta.asJson().c_str());

   // FIXME: CORRIGIR o LoggingPolicy!!!
   const auto epoch_info = EpochInfo(initial_snapshot, final_snapshot, reason, LoggingPolicy::LOGGING_DISABLED);

   m_snapshots.emplace_back(final_snapshot);
   m_epochs.emplace_back(epoch_info);

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_END, m_vd.getEpochID(), false);
   m_watchdog.refresh();

   newEpoch();
}

void EpochCntlr::notifyEpochEnding(const WatchdogEvent event, const UInt64 arg)
{
   if (event == WatchdogEvent::TIMEOUT)
   {
      Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT, static_cast<subsecond_time_t>(arg).m_time, false);
   }
   else
   {
      Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT_INS, arg, false);
   }
}
