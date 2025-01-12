#include "watchdog.h"

#include <utility>
#include "config.h"
#include "config.hpp"
#include "hooks_manager.h"
#include "simulator.h"

Watchdog::Watchdog(const SubsecondTime& timeout, const UInt64 max_instructions, WatchdogCallback subscribe) :
    m_max_interval_time(timeout),
    m_max_interval_instr(max_instructions),
    m_subscribe(std::move(subscribe))
{
   if (m_max_interval_time > SubsecondTime::Zero())
   {
      LOG_ASSERT_ERROR(m_max_interval_time >= SubsecondTime::US(1), "The max_interval_time is less than 1000ns (1μs)");

      const auto _interrupt = [](const UInt64 self, const UInt64 time) -> SInt64
      {
         reinterpret_cast<Watchdog*>(self)->interrupt(SubsecondTime::FS(time));
         return 0;
      };
      Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC, _interrupt, reinterpret_cast<UInt64>(this));
   }

   if (m_max_interval_instr > 0)
   {
      const static auto ins_per_core = Sim()->getConfig()->getHPIInstructionsPerCore();
      const static auto ins_global   = Sim()->getConfig()->getHPIInstructionsGlobal();
      LOG_ASSERT_ERROR(m_max_interval_instr >= ins_global, "'max_interval_instructions' is less than 'ins_global'");
      LOG_ASSERT_ERROR(ins_global >= ins_per_core && m_max_interval_instr % ins_global == 0, "Invalid value for 'ins_per_core' or 'ins_global'");

      const auto _interrupt = [](const UInt64 self, const UInt64 instr) -> SInt64
      {
         reinterpret_cast<Watchdog*>(self)->interrupt(instr);
         return 0;
      };
      Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC_INS, _interrupt, reinterpret_cast<UInt64>(this));
   }
}

Watchdog::~Watchdog()
{
   printf("DESTRUINDO Watchdog\n");
}

void
Watchdog::interrupt(const SubsecondTime& now)
{
   m_current.time = now;
   if (periodBetween(m_last.time, m_current.time) >= m_max_interval_time)
   {
      printf("now: %lu | last: [%lu %lu]\n", now.getNS(), m_last.time.getNS(), m_last.instr);
      m_subscribe(Event::TIMEOUT);
   }
}

void
Watchdog::interrupt(const UInt64 instr)
{
   m_current.instr = instr;
   if (periodBetween(m_last.instr, m_current.instr) >= m_max_interval_instr)
   {
      printf("ins: %lu | last: [%lu %lu]\n", instr, m_last.time.getNS(), m_last.instr);
      m_subscribe(Event::TIMEOUT_INS);
   }
}
