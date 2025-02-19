#include "hooks_manager.h"

void HooksManager::registerHook(const HookType::hook_type_t type, const HookCallbackFunc func, const UInt64 argument, const HookCallbackOrder order)
{
   m_registry[type].emplace_back(func, argument, order);
}

void HooksManager::registerHook(const HookType::hook_type_t type, const HookNewCallbackFunc& func, const HookCallbackOrder order)
{
   m_registry[type].emplace_back(func, order);
}

SInt64 HooksManager::callHooks(const HookType::hook_type_t type, const UInt64 argument, const bool expect_return)
{
   for (int order = 0; order < static_cast<int>(NUM_HOOK_ORDER); ++order)
   {
      for (auto& callback: m_registry[type])
      {
         if (callback.order == static_cast<HookCallbackOrder>(order))
         {
            if (const auto result = callback.func(argument); expect_return && result != -1)
               return result;
         }
      }
   }

   return -1;
}
