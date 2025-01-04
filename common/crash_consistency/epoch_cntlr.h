#pragma once

#include "epoch_info.h"
#include "subsecond_time.h"
#include "watchdog.h"

#include <vector>

class EpochManager;

class VersionedDomain
{
public:
   explicit VersionedDomain(const UInt32 id) :
       m_id(id), m_eid(0) {}

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
   explicit EpochCntlr(EpochManager& epoch_manager, const UInt32 vd_id,
                       const SubsecondTime& max_interval_time, UInt64 max_interval_instr,
                       const UInt32 total_cores) :
       EpochCntlr(epoch_manager, vd_id, max_interval_time, max_interval_instr,
                  generateCoresArray(0, static_cast<core_id_t>(total_cores) - 1)) {}

   explicit EpochCntlr(EpochManager& epoch_manager, const UInt32 vd_id,
                       const SubsecondTime& max_interval_time, const UInt64 max_interval_instr,
                       const core_id_t core_start, const core_id_t core_end) :
       EpochCntlr(epoch_manager, vd_id, max_interval_time, max_interval_instr,
                  generateCoresArray(core_start, core_end)) {}

   explicit EpochCntlr(EpochManager& epoch_manager, UInt32 vd_id,
                       const SubsecondTime& max_interval_time, UInt64 max_interval_instr,
                       const std::vector<core_id_t>& cores);

   ~EpochCntlr();

   void commit(CheckpointReason reason);

private:
   EpochManager& m_epoch_manager;
   VersionedDomain m_vd;
   std::vector<core_id_t> m_cores;
   Watchdog m_watchdog;
   std::unordered_map<UInt64, EpochInfo> m_epochs;

   void newEpoch();

   static std::vector<core_id_t> generateCoresArray(core_id_t core_start, core_id_t core_end);
   static void notifyEpochEnding(WatchdogEvent event, UInt64 arg);

   friend class EpochManager;
};
