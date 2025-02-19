#pragma once

#include "core_manager.h"
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

   /// callback_func, timeout
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, CoreIds&& core_ids) :
       Watchdog(callback_func, timeout, 0, core_ids) {}

   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, AllCoreIds auto... core_ids) :
       Watchdog(callback_func, timeout, {core_ids...}) {}

   /// callback_func, instruction_threshold
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   Watchdog(const WatchdogCallback& callback_func, const UInt64 instruction_threshold, CoreIds&& core_ids) :
       Watchdog(callback_func, SubsecondTime::Zero(), instruction_threshold, core_ids) {}

   Watchdog(const WatchdogCallback& callback_func, const UInt64 instruction_threshold, AllCoreIds auto... core_ids) :
       Watchdog(callback_func, instruction_threshold, {core_ids...}) {}

   /// callback_func, timeout, instruction_threshold
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, const UInt64 instruction_threshold,
            CoreIds&& core_ids) :
       m_max_interval_time(timeout),
       m_max_interval_instr(instruction_threshold),
       m_cores(std::vector(core_ids)),
       m_callback_func(callback_func)
   {
      WatchdogEventBase::registerWatchdog(this);
   }

   Watchdog(const WatchdogCallback& callback_func, const SubsecondTime& timeout, const UInt64 instruction_threshold,
            AllCoreIds auto... core_ids) :
       Watchdog(callback_func, timeout, instruction_threshold, {core_ids...}) {}

   ~Watchdog()
   {
      WatchdogEventBase::removeWatchdog(this);
   }

   Watchdog(const Watchdog&)            = delete;
   Watchdog(Watchdog&&)                 = delete;
   Watchdog& operator=(const Watchdog&) = delete;
   Watchdog& operator=(Watchdog&&)      = delete;

   void refresh();

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
   void check() const;

   [[nodiscard]] bool timeoutEnabled() const { return m_max_interval_time > SubsecondTime::Zero(); }
   [[nodiscard]] bool maxInstructionsEnabled() const { return m_max_interval_instr > 0; }

   template<typename T>
   static T periodBetween(const T last, const T current)
   {
      return current >= last ? current - last : last - current;
   }

   friend class WatchdogEventBase;
};

using WatchdogEvent = Watchdog::Event;
