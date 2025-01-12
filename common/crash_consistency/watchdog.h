#pragma once

#include "subsecond_time.h"
#include <functional>

class Watchdog
{
public:
   enum class Event
   {
      TIMEOUT,
      TIMEOUT_INS
   };
   using WatchdogCallback = std::function<void(Event)>;

   Watchdog(const SubsecondTime& timeout, const WatchdogCallback& subscribe) :
       Watchdog(timeout, 0, subscribe) {}

   Watchdog(const UInt64 max_instructions, const WatchdogCallback& subscribe) :
       Watchdog(SubsecondTime::Zero(), max_instructions, subscribe) {}

   Watchdog(const SubsecondTime& timeout, UInt64 max_instructions, WatchdogCallback subscribe);

   ~Watchdog();

   Watchdog(const Watchdog&)            = delete;
   Watchdog(Watchdog&&)                 = delete;
   Watchdog& operator=(const Watchdog&) = delete;
   Watchdog& operator=(Watchdog&&)      = delete;

   void refresh() { m_last = m_current; }

private:
   struct instant_t
   {
      SubsecondTime time{ SubsecondTime::Zero() };
      UInt64 instr{ 0 };
   };

   SubsecondTime m_max_interval_time;
   UInt64 m_max_interval_instr;
   const std::function<void(Event)> m_subscribe;

   instant_t m_last, m_current;

   void interrupt(const SubsecondTime& now);
   void interrupt(UInt64 instr);

   template<typename T>
   static T periodBetween(T last, T current)
   {
      return current >= last ? current - last : last - current;
   }
};

using WatchdogEvent = Watchdog::Event;
