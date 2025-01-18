#pragma once

#include "epoch_cntlr.h"
#include "subsecond_time.h"
#include "watchdog.h"
#include <memory>
#include <optional>

class VersionedDomain {
public:
   explicit VersionedDomain(UInt32 id) : m_id(id), m_eid(id) {}
   ~VersionedDomain() = default;

   void increment() { m_eid++; }

   [[nodiscard]] UInt32 getID() const { return m_id; }
   [[nodiscard]] UInt64 getEpochID() const { return m_eid; }

private:
   const UInt32 m_id;
   UInt64 m_eid;
};

class EpochManager
{
public:
   EpochManager();
   ~EpochManager();

   EpochManager(const EpochManager&)            = delete;
   EpochManager(EpochManager&&)                 = delete;
   EpochManager& operator=(const EpochManager&) = delete;
   EpochManager& operator=(EpochManager&&)      = delete;

   [[nodiscard]] EpochCntlr& getEpochCntlr(core_id_t core_id) const { return *m_cntlr; }

//   [[nodiscard]] UInt64 getSystemEID() const { return 0; }
//   [[nodiscard]] UInt64 getSystemEID(core_id_t core_id) const { return 0; }

   friend class EpochCntlr;

private:
   std::unique_ptr<EpochCntlr> m_cntlr;
   std::unique_ptr<Watchdog> m_watchdog;
   std::optional<SubsecondTime> m_max_interval_time;
   std::optional<UInt64> m_max_interval_instr;

   void start();
   void end();

   static std::optional<SubsecondTime> getMaxIntervalTime();
   static std::optional<UInt64> getMaxIntervalInstructions();
};
