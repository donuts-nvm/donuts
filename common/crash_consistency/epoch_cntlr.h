#pragma once

#include "checkpoint_reason.h"
#include "epoch_info.h"
#include "subsecond_time.h"
#include "system_snapshot.h"
#include "watchdog.h"

#include <vector>

class EpochManager;

class VersionedDomain
{
public:
   explicit VersionedDomain(UInt32 id, core_id_t first_core, core_id_t last_core);

   UInt64 increment() { return ++m_eid; }

   [[nodiscard]] UInt32 getID() const { return m_id; }
   [[nodiscard]] UInt64 getEpochID() const { return m_eid; }

private:
   const UInt32 m_id;
   UInt64 m_eid;
};

class EpochCntlr
{
public:
   explicit EpochCntlr(EpochManager& epoch_manager,
                       UInt32 vd_id,
                       core_id_t first_core, core_id_t last_core,
                       const SubsecondTime& max_interval_time, UInt64 max_interval_instr,
                       const std::vector<core_id_t>& cores);

   void commit(CheckpointReason reason);

   [[nodiscard]] UInt64 getCurrentEID() const { return m_vd.getEpochID(); }

private:
   EpochManager& m_epoch_manager;
   VersionedDomain m_vd;
   std::vector<core_id_t> m_cores;
   Watchdog m_watchdog;
   std::vector<SystemSnapshot> m_snapshots;
   std::vector<EpochInfo> m_epochs;

   void newEpoch();

   static void notifyEpochEnding(WatchdogEvent event, UInt64 arg);

   friend class EpochManager;
};
