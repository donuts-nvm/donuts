#pragma once

#include "subsecond_time.h"
#include "thread_manager.h"

#include <unordered_map>
#include <vector>
#include <functional>

class HookType
{
public:
   enum hook_type_t {
      // Hook name                Parameter (cast from UInt64)         Description
      HOOK_PERIODIC,              // SubsecondTime current_time        Barrier was reached
      HOOK_PERIODIC_INS,          // UInt64 icount                     Instruction-based periodic callback
      HOOK_SIM_START,             // none                              Simulation start
      HOOK_SIM_END,               // none                              Simulation end
      HOOK_ROI_BEGIN,             // none                              ROI begin
      HOOK_ROI_END,               // none                              ROI end
      HOOK_CPUFREQ_CHANGE,        // UInt64 coreid                     CPU frequency was changed
      HOOK_MAGIC_MARKER,          // MagicServer::MagicMarkerType *    Magic marker (SimMarker) in application
      HOOK_MAGIC_USER,            // MagicServer::MagicMarkerType *    Magic user function (SimUser) in application
      HOOK_INSTR_COUNT,           // UInt64 coreid                     Core has executed a preset number of instructions
      HOOK_THREAD_CREATE,         // HooksManager::ThreadCreate        Thread creation
      HOOK_THREAD_START,          // HooksManager::ThreadTime          Thread start
      HOOK_THREAD_EXIT,           // HooksManager::ThreadTime          Thread end
      HOOK_THREAD_STALL,          // HooksManager::ThreadStall         Thread has entered stalled state
      HOOK_THREAD_RESUME,         // HooksManager::ThreadResume        Thread has entered running state
      HOOK_THREAD_MIGRATE,        // HooksManager::ThreadMigrate       Thread was moved to a different core
      HOOK_INSTRUMENT_MODE,       // UInt64 Instrument Mode            Simulation mode change (ex. detailed, ffwd)
      HOOK_PRE_STAT_WRITE,        // const char * prefix               Before statistics are written (update generated stats now!)
      HOOK_SYSCALL_ENTER,         // SyscallMdl::HookSyscallEnter      Thread enters a system call
      HOOK_SYSCALL_EXIT,          // SyscallMdl::HookSyscallExit       Thread exist from system call
      HOOK_APPLICATION_START,     // app_id_t                          Application (re)start
      HOOK_APPLICATION_EXIT,      // app_id_t                          Application exit
      HOOK_APPLICATION_ROI_BEGIN, // none                              ROI begin, always triggers
      HOOK_APPLICATION_ROI_END,   // none                              ROI end, always triggers
      HOOK_SIGUSR1,               // none                              Sniper process received SIGUSR1
      HOOK_EPOCH_START,           // Added by Kleber Kruger            An epoch starts
      HOOK_EPOCH_END,             // Added by Kleber Kruger            An epoch ends
      HOOK_EPOCH_PERSISTED,       // Added by Kleber Kruger            An epoch persisted
      HOOK_EPOCH_TIMEOUT,         // Added by Kleber Kruger            An epoch timeout
      HOOK_EPOCH_TIMEOUT_INS,     // Added by Kleber Kruger            An epoch timeout (by instruction interval)
      HOOK_TYPES_MAX
   };

   inline static const char* hook_type_names[] = {
      "HOOK_PERIODIC",
      "HOOK_PERIODIC_INS",
      "HOOK_SIM_START",
      "HOOK_SIM_END",
      "HOOK_ROI_BEGIN",
      "HOOK_ROI_END",
      "HOOK_CPUFREQ_CHANGE",
      "HOOK_MAGIC_MARKER",
      "HOOK_MAGIC_USER",
      "HOOK_INSTR_COUNT",
      "HOOK_THREAD_CREATE",
      "HOOK_THREAD_START",
      "HOOK_THREAD_EXIT",
      "HOOK_THREAD_STALL",
      "HOOK_THREAD_RESUME",
      "HOOK_THREAD_MIGRATE",
      "HOOK_INSTRUMENT_MODE",
      "HOOK_PRE_STAT_WRITE",
      "HOOK_SYSCALL_ENTER",
      "HOOK_SYSCALL_EXIT",
      "HOOK_APPLICATION_START",
      "HOOK_APPLICATION_EXIT",
      "HOOK_APPLICATION_ROI_BEGIN",
      "HOOK_APPLICATION_ROI_END",
      "HOOK_SIGUSR1",
      "HOOK_EPOCH_START",
      "HOOK_EPOCH_END",
      "HOOK_EPOCH_PERSISTED",
      "HOOK_EPOCH_TIMEOUT",
      "HOOK_EPOCH_TIMEOUT_INS"
   };

   static_assert(std::size(hook_type_names) == static_cast<std::size_t>(HOOK_TYPES_MAX),
                 "Mismatch in HookType::hook_type_t and HookType::hook_type_names");
};

namespace std {
template <> struct hash<HookType::hook_type_t> {
   size_t operator()(const HookType::hook_type_t & type) const noexcept {
      return static_cast<size_t>(type);
   }
};
}

class HooksManager
{
public:
   enum HookCallbackOrder {
      ORDER_NOTIFY_PRE,    // For callbacks that want to inspect state before any actions
      ORDER_ACTION,        // For callbacks that want to change simulator state based on the event
      ORDER_NOTIFY_POST,   // For callbacks that want to inspect state after any actions
      NUM_HOOK_ORDER,
   };

   using HookCallbackFunc    = SInt64 (*)(UInt64, UInt64);
   using HookNewCallbackFunc = std::function<SInt64(UInt64)>;

   struct HookCallback
   {
      HookNewCallbackFunc func;
      UInt64 obj;
      HookCallbackOrder order;

      HookCallback(HookCallbackFunc _func, UInt64 _obj, HookCallbackOrder _order);
      HookCallback(const HookNewCallbackFunc& _func, HookCallbackOrder _order);
   };

   struct ThreadCreate {
      thread_id_t thread_id{};
      thread_id_t creator_thread_id{};
   };
   struct ThreadTime {
      thread_id_t thread_id{};
      subsecond_time_t time{};
   };
   struct ThreadStall {
      thread_id_t thread_id{};
      ThreadManager::stall_type_t reason{};
      subsecond_time_t time{};
   };
   struct ThreadResume {
      thread_id_t thread_id{};
      thread_id_t thread_by{};
      subsecond_time_t time{};
   };
   struct ThreadMigrate {
      thread_id_t thread_id{};
      core_id_t core_id{};
      subsecond_time_t time{};
   };

   void init();
   void fini();

   void registerHook(HookType::hook_type_t type, HookCallbackFunc func, UInt64 argument, HookCallbackOrder order = ORDER_NOTIFY_PRE);

   void registerHook(HookType::hook_type_t type, std::invocable<UInt64> auto&& func, HookCallbackOrder order = ORDER_NOTIFY_PRE)
      requires(std::convertible_to<std::invoke_result_t<decltype(func), UInt64>, SInt64> ||
               std::same_as<std::invoke_result_t<decltype(func), UInt64>, void>)
   {
      auto wrapper = [f = std::forward<decltype(func)>(func)](UInt64 arg) -> SInt64 {
         if constexpr (std::same_as<std::invoke_result_t<decltype(f), UInt64>, void>) {
            f(arg);
            return -1L;
         } else {
            return static_cast<SInt64>(f(arg));
         }
      };
      m_registry[type].emplace_back(std::move(wrapper), order);
   }

   SInt64 callHooks(HookType::hook_type_t type, UInt64 argument, bool expect_return = false);

private:
   std::unordered_map<HookType::hook_type_t, std::vector<HookCallback>> m_registry{};
};
