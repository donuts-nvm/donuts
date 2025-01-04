#include "watchdog.h"
#include "config.h"
#include "hooks_manager.h"
#include "simulator.h"

#include <core_manager.h>
#include <utility>

void
WatchdogEventBase::registerWatchdog(Watchdog* watchdog)
{
   std::lock_guard lock(mutex);

   // auto registerHookIfEmpty = [](auto& container, auto wg, auto hookType, auto converter)
   // {
   //    if (container.empty()) {
   //       Sim()->getHooksManager()->registerHook(hookType, [&container, converter](const UInt64, const UInt64 value) -> SInt64 {
   //          for (const auto& it: container) it->interrupt(converter(value));
   //          return 0;
   //       }, 0);
   //    }
   //    container.push_back(wg);
   // };

   if (watchdog->timeoutEnabled())
   {
      const static auto barrier_interval = Sim()->getClockSkewMinimizationServer()->getBarrierInterval();
      LOG_ASSERT_ERROR(watchdog->m_max_interval_time >= SubsecondTime::US(1), "The max_interval_time is less than 1000ns (1μs)");
      LOG_ASSERT_ERROR(watchdog->m_max_interval_time.getFS() % barrier_interval.getFS() == 0, "Invalid value for 'barrier_interval'");

      if (instances_by_time.empty())
      {
         const auto _interrupt = [](const UInt64, const UInt64 time) -> SInt64
         {
            for (const auto& it: instances_by_time) it->interrupt(SubsecondTime::FS(time));
            return 0;
         };
         Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC, _interrupt, 0);
      }
      instances_by_time.push_back(watchdog);

      // registerHookIfEmpty(instances_by_time, watchdog, HookType::HOOK_PERIODIC, [](const UInt64 time)
      //                     { return SubsecondTime::FS(time); });
   }

   if (watchdog->instructionThresholdEnabled())
   {
      const static auto ins_per_core = Sim()->getConfig()->getHPIInstructionsPerCore();
      const static auto ins_global   = Sim()->getConfig()->getHPIInstructionsGlobal();
      LOG_ASSERT_ERROR(watchdog->m_max_interval_instr >= ins_global, "'max_interval_instructions' is less than 'ins_global'");
      LOG_ASSERT_ERROR(ins_global >= ins_per_core && watchdog->m_max_interval_instr % ins_global == 0, "Invalid value for 'ins_per_core' or 'ins_global'");

      if (instances_by_instr.empty())
      {
         const auto _interrupt = [](const UInt64, const UInt64 instr) -> SInt64
         {
            for (const auto& it: instances_by_instr) it->interrupt(instr);
            return 0;
         };
         Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC_INS, _interrupt, 0);
      }
      instances_by_instr.push_back(watchdog);

      // registerHookIfEmpty(instances_by_instr, watchdog, HookType::HOOK_PERIODIC_INS, [](const UInt64 instr)
      //                     { return instr; });
   }

   printf("Watchdog created: %p\n", watchdog);
}

void
WatchdogEventBase::removeWatchdog(Watchdog* watchdog)
{
   std::lock_guard lock(mutex);

   if (const auto it = std::ranges::find(instances_by_time, watchdog);
       it != instances_by_time.end())
   {
      instances_by_time.erase(it);
      printf("Watchdog destroyed from instances_by_time: %p\n", watchdog);
   }
   if (const auto it = std::ranges::find(instances_by_instr, watchdog);
       it != instances_by_instr.end())
   {
      instances_by_instr.erase(it);
      printf("Watchdog destroyed instances_by_instr: %p\n", watchdog);
   }

   // auto removeInstance = [](auto& container, const Watchdog* watchdog) {
   //    if (const auto it = std::ranges::find(container, watchdog); it != container.end())
   //       container.erase(it);
   // };
   // removeInstance(instances_by_time, watchdog);
   // removeInstance(instances_by_instr, watchdog);
}

Watchdog::Watchdog(const std::function<void(Event, UInt64)>& callback_func,
                   const SubsecondTime& timeout, const UInt64 instruction_threshold, const std::vector<core_id_t>& cores) :
    m_max_interval_time(timeout),
    m_max_interval_instr(instruction_threshold),
    m_cores(cores),
    m_callback_func(callback_func)
{
   WatchdogEventBase::registerWatchdog(this);
}
Watchdog::~Watchdog()
{
   WatchdogEventBase::removeWatchdog(this);
}

void
Watchdog::interrupt(const SubsecondTime& now)
{
   m_current.time = now;
   if (instructionThresholdEnabled()) m_current.instr = Sim()->getCoreManager()->getInstructionCount(m_cores);

   // if (const auto gap = periodBetween(m_last.time, m_current.time);
   //     gap >= m_max_interval_time)
   // {
   //    printf("now: %lu | last: [t: %lu i: %lu] | gap: %lu\n", now.getNS(), m_last.time.getNS(), m_last.instr, gap.getNS());
   //    m_callback_func(Event::TIMEOUT, static_cast<subsecond_time_t>(gap).m_time);
   // }
   check();
}

void
Watchdog::interrupt(const UInt64 instr)
{
   m_current.instr = instr;
   if (timeoutEnabled()) m_current.time = Sim()->getClockSkewMinimizationServer()->getGlobalTime();

   // if (const auto gap = periodBetween(m_last.instr, m_current.instr);
   //     gap >= m_max_interval_instr)
   // {
   //    printf("ins: %lu | last: [t: %lu i: %lu] | gap: %lu\n", instr, m_last.time.getNS(), m_last.instr, gap);
   //    m_callback_func(Event::TIMEOUT_INS, gap);
   // }
   check();
}

void
Watchdog::check() const
{
   // printf("current { time: %lu, instr: %lu } | last: { time: %lu, instr: %lu }\n", m_current.time.getNS(), m_current.instr, m_last.time.getNS(), m_last.instr);

   if (timeoutEnabled())
   {
      if (const auto gap = periodBetween(m_last.time, m_current.time);
          gap >= m_max_interval_time)
      {
         // printf("NOW: %lu | last { time: %lu instr: %lu } | GAP: %lu\n", m_current.time.getNS(), m_last.time.getNS(), m_last.instr, gap.getNS());
         m_callback_func(Event::TIMEOUT, static_cast<subsecond_time_t>(gap).m_time);
      }
   }
   if (instructionThresholdEnabled())
   {
      if (const auto gap = periodBetween(m_last.instr, m_current.instr);
          gap >= m_max_interval_instr)
      {
         // printf("INS: %lu | last { time: %lu, instr: %lu } | GAP: %lu\n", m_current.instr, m_last.time.getNS(), m_last.instr, gap);
         m_callback_func(Event::TIMEOUT_INS, gap);
      }
   }
}

void
Watchdog::refresh()
{
   m_current.time  = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
   m_current.instr = Sim()->getCoreManager()->getInstructionCount(m_cores);
   m_last          = m_current;
}
