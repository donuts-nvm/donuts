#pragma once

#include "epoch_cntlr.h"
#include "subsecond_time.h"
#include "watchdog.h"
#include <memory>
#include <optional>

class EpochManager
{
public:
   EpochManager();
   ~EpochManager();

   EpochManager(const EpochManager&)            = delete;
   EpochManager(EpochManager&&)                 = delete;
   EpochManager& operator=(const EpochManager&) = delete;
   EpochManager& operator=(EpochManager&&)      = delete;

private:
   std::unique_ptr<EpochCntlr> m_cntlr;
   std::unique_ptr<Watchdog> m_watchdog;
   // Watchdog* m_watchdog;
   std::optional<SubsecondTime> m_max_interval_time;
   std::optional<UInt64> m_max_interval_instr;

   void start();
   void end();
   void handleWatchdog(WatchdogEvent event);

   static std::optional<SubsecondTime> getMaxIntervalTime();
   static std::optional<UInt64> getMaxIntervalInstructions();
};
