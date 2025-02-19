#include "hooks_manager.h"

HooksManager::HookCallback::HookCallback(const HookCallbackFunc _func, const UInt64 _obj, const HookCallbackOrder _order) :
    func([_func, _obj](const UInt64 arg2){
       return _func(_obj, arg2);
    }), obj(_obj), order(_order) {}

HooksManager::HookCallback::HookCallback(const std::function<SInt64(UInt64)>& _func, const HookCallbackOrder _order) :
    func(_func), obj(0), order(_order) {}

void HooksManager::registerHook(const HookType::hook_type_t type, const HookCallbackFunc func, const UInt64 argument, const HookCallbackOrder order)
{
   m_registry[type].emplace_back(func, argument, order);
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
