#include "epoch_manager.h"
#include "config.hpp"
#include "hooks_manager.h"
#include "simulator.h"

EpochManager::EpochManager() :
    m_watchdog(nullptr),
    m_max_interval_time(getMaxIntervalTime()),
    m_max_interval_instr(getMaxIntervalInstructions())
{
   const auto _start = [](const UInt64 self, UInt64) -> SInt64
   {
      reinterpret_cast<EpochManager*>(self)->start();
      return 0;
   };
   const auto _exit = [](const UInt64 self, UInt64) -> SInt64
   {
      reinterpret_cast<EpochManager*>(self)->end();
      return 0;
   };
   const auto this_ptr = reinterpret_cast<UInt64>(this);

   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_START, _start, this_ptr);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_EXIT, _exit, this_ptr);

   m_cntlr = std::make_unique<EpochCntlr>(*this);
}

EpochManager::~EpochManager() = default;

void
EpochManager::end()
{
   printf("EXITING APPLICATION...\n");
}

void
EpochManager::start()
{
   const auto max_interval_time  = m_max_interval_time.value_or(SubsecondTime::Zero());
   const auto max_interval_instr = m_max_interval_instr.value_or(0L);

   const auto _watchdog_timeout = [this](const WatchdogEvent event)
   {
      m_watchdog->refresh();
      const auto e = event == WatchdogEvent::TIMEOUT ? HookType::HOOK_EPOCH_TIMEOUT :
                                                       HookType::HOOK_EPOCH_TIMEOUT_INS;
      Sim()->getHooksManager()->callHooks(e, 0, false);
   };
   m_watchdog = std::make_unique<Watchdog>(max_interval_time, max_interval_instr, _watchdog_timeout);

   if (m_max_interval_time)
   {
      const auto _timeout = [](const UInt64 self, const UInt64 eid) -> SInt64
      {
         printf("CHECKPOINTING BY TIMEOUT [%lu]\n", eid);
         return 0;
      };
      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT, _timeout, reinterpret_cast<UInt64>(this));
   }

   if (m_max_interval_instr)
   {
      const auto _timeout_ins = [](const UInt64 self, const UInt64 eid) -> SInt64
      {
         printf("CHECKPOINTING BY INSTRUCTION [%lu]\n", eid);
         return 0;
      };
      Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS, _timeout_ins, reinterpret_cast<UInt64>(this));
   }
}

void
EpochManager::handleWatchdog(WatchdogEvent event)
{
   printf("Checkpoint by %i\n", static_cast<int>(event));
   m_watchdog->refresh();
}

std::optional<SubsecondTime>
EpochManager::getMaxIntervalTime()
{
   const String key = "epoch/max_interval_time";

   const auto max_interval_time = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   LOG_ASSERT_ERROR(max_interval_time >= 0, "Negative value for max_interval_time");
   LOG_ASSERT_ERROR(max_interval_time == 0 || max_interval_time >= 1000, "The max_interval_time is less than 1000ns (1μs)");

   return max_interval_time != 0 ? std::optional{ SubsecondTime::NS(max_interval_time) } : std::nullopt;
}

std::optional<UInt64>
EpochManager::getMaxIntervalInstructions()
{
   const String key = "epoch/max_interval_instructions";

   const auto max_interval_instr = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   LOG_ASSERT_ERROR(max_interval_instr >= 0, "Negative value for 'max_interval_instructions'");

   return max_interval_instr != 0 ? std::optional{ max_interval_instr } : std::nullopt;
}
