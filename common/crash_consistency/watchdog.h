#pragma once

#include "subsecond_time.h"
#include <functional>
#include <mutex>
#include <vector>

class Watchdog;

class WatchdogEventBase
{
   static void registerWatchdog(Watchdog* watchdog);
   static void removeWatchdog(Watchdog* watchdog);

   static inline std::vector<Watchdog*> instances_by_time;
   static inline std::vector<Watchdog*> instances_by_instr;
   static inline std::mutex mutex;

   friend class Watchdog;
};

class Watchdog
{
public:
   enum class Event
   {
      TIMEOUT,
      TIMEOUT_INS
   };
   using WatchdogCallback = std::function<void(Event, UInt64 arg)>;

   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout) :
       Watchdog(callback_func, timeout, 0, std::vector<core_id_t>{}) {}

   template<typename... Cores>
   Watchdog(const WatchdogCallback& callback_func, const UInt64 max_instructions, Cores... cores) :
       Watchdog(callback_func, SubsecondTime::Zero(), max_instructions, std::vector<core_id_t>{ cores... }) {}

   Watchdog(const WatchdogCallback& callback_func, const UInt64 max_instructions, const std::vector<core_id_t>& cores) :
       Watchdog(callback_func, SubsecondTime::Zero(), max_instructions, cores) {}

   template<typename... Cores>
   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, const UInt64 max_instructions, Cores... cores) :
       Watchdog(callback_func, timeout, max_instructions, std::vector<core_id_t>{ cores... }) {}

   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, UInt64 max_instructions, const std::vector<core_id_t>& cores);

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
   std::vector<core_id_t> m_cores;
   WatchdogCallback m_callback_func;

   instant_t m_last, m_current;

   void interrupt(const SubsecondTime& now);
   void interrupt(UInt64 instr);

   template<typename T>
   static T periodBetween(T last, T current)
   {
      return current >= last ? current - last : last - current;
   }

   friend class WatchdogEventBase;
   static inline WatchdogEventBase event_base;
};

using WatchdogEvent = Watchdog::Event;
