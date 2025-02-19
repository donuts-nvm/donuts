#include "watchdog.h"
#include "config.h"
#include "core_manager.h"
#include "hooks_manager.h"
#include "simulator.h"

#include <utility>

void WatchdogEventBase::registerWatchdog(Watchdog* watchdog)
{
   std::lock_guard lock(mutex);

   if (watchdog->timeoutEnabled())
   {
      const static auto barrier_interval = Sim()->getClockSkewMinimizationServer()->getBarrierInterval();
      LOG_ASSERT_ERROR(watchdog->m_max_interval_time >= SubsecondTime::US(1), "The max_interval_time is less than 1000ns (1μs)");
      LOG_ASSERT_ERROR(watchdog->m_max_interval_time.getFS() % barrier_interval.getFS() == 0, "Invalid value for 'barrier_interval'");

      if (instances_by_time.empty())
      {
         Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC,
                                                [](const UInt64 time) {
                                                   for (const auto& it: instances_by_time)
                                                      it->interrupt(SubsecondTime::FS(time));
                                                });
      }
      instances_by_time.push_back(watchdog);
   }

   if (watchdog->maxInstructionsEnabled())
   {
      const static auto ins_per_core = Sim()->getConfig()->getHPIInstructionsPerCore();
      const static auto ins_global   = Sim()->getConfig()->getHPIInstructionsGlobal();
      LOG_ASSERT_ERROR(watchdog->m_max_interval_instr >= ins_global, "'max_interval_instructions' is less than 'ins_global'");
      LOG_ASSERT_ERROR(ins_global >= ins_per_core && watchdog->m_max_interval_instr % ins_global == 0, "Invalid value for 'ins_per_core' or 'ins_global'");

      if (instances_by_instr.empty())
      {
         Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC_INS,
                                                [](const UInt64 instr) {
                                                   for (const auto& it: instances_by_instr)
                                                      it->interrupt(instr);
                                                });
      }
      instances_by_instr.push_back(watchdog);
   }
}

void WatchdogEventBase::removeWatchdog(Watchdog* watchdog)
{
   std::lock_guard lock(mutex);

   if (const auto it = std::ranges::find(instances_by_time, watchdog);
       it != instances_by_time.end())
   {
      instances_by_time.erase(it);
   }
   if (const auto it = std::ranges::find(instances_by_instr, watchdog);
       it != instances_by_instr.end())
   {
      instances_by_instr.erase(it);
   }
}

void Watchdog::interrupt(const SubsecondTime& now)
{
   m_current.time = now;
   if (maxInstructionsEnabled()) m_current.instr = Sim()->getCoreManager()->getInstructionCount(m_cores);
   check();
}

void Watchdog::interrupt(const UInt64 instr)
{
   m_current.instr = instr;
   if (timeoutEnabled()) m_current.time = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
   check();
}

void Watchdog::check() const
{
   if (timeoutEnabled())
   {
      if (const auto gap = periodBetween(m_last.time, m_current.time);
          gap >= m_max_interval_time)
      {
         // printf("NOW: %lu | last { time: %lu instr: %lu } | GAP: %lu\n", m_current.time.getNS(), m_last.time.getNS(), m_last.instr, gap.getNS());
         m_callback_func(Event::TIMEOUT, static_cast<subsecond_time_t>(gap).m_time);
      }
   }
   if (maxInstructionsEnabled())
   {
      if (const auto gap = periodBetween(m_last.instr, m_current.instr);
          gap >= m_max_interval_instr)
      {
         // printf("INS: %lu | last { time: %lu, instr: %lu } | GAP: %lu\n", m_current.instr, m_last.time.getNS(), m_last.instr, gap);
         m_callback_func(Event::TIMEOUT_INS, gap);
      }
   }
}

void Watchdog::refresh()
{
   m_current.time  = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
   m_current.instr = Sim()->getCoreManager()->getInstructionCount(m_cores);
   m_last          = m_current;
}
