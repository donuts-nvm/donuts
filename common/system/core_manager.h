#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include "fixed_types.h"
#include "tls.h"
#include "lock.h"
#include "log.h"
#include "core.h"
#include "config.h"

#include <concepts>
#include <fstream>
#include <map>
#include <numeric>
#include <ranges>
#include <vector>

template<typename R>
concept CoreIdRange = std::ranges::input_range<R> && std::same_as<std::ranges::range_value_t<R>, core_id_t>;

template<typename... Args>
concept AllCoreIds = (std::same_as<Args, core_id_t> && ...);

template<template<typename, typename, typename...> class MapType = std::map,
         typename CoreRange,
         typename Func>
   requires CoreIdRange<std::remove_cvref_t<CoreRange>> && std::invocable<Func, core_id_t>
auto mapCoreIdsTo(CoreRange&& core_ids, Func&& func)
{
   using ValueType = std::invoke_result_t<Func, core_id_t>;
   MapType<core_id_t, ValueType> result;

   const auto total_cores = static_cast<core_id_t>(Config::getSingleton()->getTotalCores());

   if (std::ranges::empty(core_ids))
   {
      for (auto core_id : std::ranges::iota_view{ 0, total_cores })
      {
         result.emplace(core_id, func(core_id));
      }
   }
   else
   {
      for (auto& core_id : core_ids)
      {
         LOG_ASSERT_ERROR(core_id >= 0 && core_id < total_cores, "cores[%d] out of range", core_id);
         result.emplace(core_id, func(core_id));
      }
   }

   return result;
}

class Core;

class CoreManager
{
   public:
      CoreManager();
      ~CoreManager();

      enum ThreadType {
          INVALID,
          APP_THREAD,   // Application (Pin) thread
          CORE_THREAD,  // Core (Performance model) thread
          SIM_THREAD    // Simulator (Network model) thread
      };

      void initializeCommId(SInt32 comm_id);
      void initializeThread(core_id_t core_id);
      void terminateThread();
      core_id_t registerSimThread(ThreadType type);

      core_id_t getCurrentCoreID(int threadIndex = -1) // id of currently active core (or INVALID_CORE_ID)
      {
         Core *core = getCurrentCore(threadIndex);
         if (!core)
             return INVALID_CORE_ID;
         else
             return core->getId();
      }
      Core *getCurrentCore(int threadIndex = -1)
      {
          return m_core_tls->getPtr<Core>(threadIndex);
      }

      Core *getCoreFromID(core_id_t core_id);

      template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
      UInt64 getInstructionCount(CoreIds&& core_ids) const
      {
         if (std::ranges::empty(core_ids)) {
            return std::accumulate(m_cores.begin(), m_cores.end(), 0,
                        [](const UInt64 sum, Core* core)
                        {
                           return sum + core->getInstructionCount();
                        });
         }
         return std::accumulate(core_ids.begin(), core_ids.end(), 0,
                     [this](const UInt64 sum, const core_id_t id)
                     {
                        LOG_ASSERT_ERROR(id >= 0 && id < static_cast<core_id_t>(Config::getSingleton()->getTotalCores()),
                                         "Illegal index [%d] in getInstructionCount!", id);
                        return sum + m_cores[id]->getInstructionCount();
                     });
      }

      UInt64 getInstructionCount(AllCoreIds auto... core_ids) const
      {
         return getInstructionCount({core_ids...});
      }

      bool amiUserThread();
      bool amiCoreThread();
      bool amiSimThread();
   private:

      UInt32 *tid_map;
      TLS *m_core_tls;
      TLS *m_thread_type_tls;

      UInt32 m_num_registered_sim_threads;
      UInt32 m_num_registered_core_threads;
      Lock m_num_registered_threads_lock;

      std::vector<Core*> m_cores;
};

#endif
