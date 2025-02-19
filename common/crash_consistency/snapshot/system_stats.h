#pragma once

#include "core_manager.h"
#include "shmem_perf_model.h"
#include "simulator.h"
#include "stats.h"
#include "system_stats_base.h"

namespace stats::detail
{

inline Core* resolveCore(const std::optional<core_id_t> core_id = std::nullopt)
{
   const auto core_manager = Sim()->getCoreManager();
   if (core_id) return core_manager->getCoreFromID(*core_id);

   const auto current_core = core_manager->getCurrentCore();
   return current_core ? current_core : core_manager->getCoreFromID(0);
}

inline SubsecondTime getElapsedTime(const ShmemPerfModel::Thread_t thread,
                                    const std::optional<core_id_t> core_id = std::nullopt)
{
   return resolveCore(core_id)->getShmemPerfModel()->getElapsedTime(thread);
}

template<typename... Args>
concept AllMemDevices = sizeof...(Args) >= 1 && (std::is_convertible_v<Args, const char*> && ...);

inline const char* accessTypeToString(const DramCntlrInterface::access_t access_type)
{
   switch (access_type)
   {
      case DramCntlrInterface::READ: return "reads";
      case DramCntlrInterface::WRITE: return "writes";
      case DramCntlrInterface::LOG: return "logs";
      default: LOG_PRINT_ERROR("Unrecognized access type");
   }
}

UInt64 getMemAccess(const DramCntlrInterface::access_t access_type, AllMemDevices auto... mem_devices)
{
   const char* param = accessTypeToString(access_type);
   UInt64 result     = 0;

   for (const auto* mem_device: std::array<const char*, sizeof...(mem_devices)>{ mem_devices... })
   {
      for (UInt32 i = 0; i < Sim()->getConfig()->getTotalCores(); i++)
      {
         auto* const metric = Sim()->getStatsManager()->getMetricObject(mem_device, i, param);
         result += metric ? metric->recordMetric() : 0;
      }
   }

   return result;
}

MemAccessCountMap getMemAccess(AllMemDevices auto... mem_devices)
{
   MemAccessCountMap mem_access;
   for (const auto* mem_device: std::array<const char*, sizeof...(mem_devices)>{ mem_devices... })
   {
      for (int i = 0; i < DramCntlrInterface::access_t::NUM_ACCESS_TYPES; i++)
      {
         const auto type = static_cast<DramCntlrInterface::access_t>(i);
         mem_access[type] += getMemAccess(type, mem_device);
      }
   }
   return mem_access;
}

MemAccessCountSummary getMemAccessSummary(AllMemDevices auto... mem_devices)
{
   const auto access_map = getMemAccess(mem_devices...);
   UInt64 total          = std::accumulate(access_map.begin(), access_map.end(), 0,
                                           [](UInt64 sum, const auto& e) { return sum + e.second; });
   return { total, access_map };
}

constexpr DramInfo summaryToDramInfo(const MemAccessCountSummary& summary)
{
   const auto [total, access_map] = summary;
   return {
      .reads    = access_map.at(DramCntlrInterface::access_t::READ),
      .writes   = access_map.at(DramCntlrInterface::access_t::WRITE),
      .accesses = total
   };
}

constexpr NvmInfo summaryToNvmInfo(const MemAccessCountSummary& summary, const UInt64 log_flushes)
{
   const auto [total_access, access_map] = summary;
   return {
      .reads       = access_map.at(DramCntlrInterface::access_t::READ),
      .writes      = access_map.at(DramCntlrInterface::access_t::WRITE),
      .logs        = access_map.at(DramCntlrInterface::access_t::LOG),
      .accesses    = total_access,
      .log_flushes = log_flushes
   };
}

}// namespace stats::detail

namespace stats
{

/// Get Global Time
inline SubsecondTime getGlobalTime()
{
   return Sim()->getClockSkewMinimizationServer()->getGlobalTime();
}

/// Get Sim Time
inline SubsecondTime getSimTime(const std::optional<core_id_t> core_id = std::nullopt)
{
   return detail::getElapsedTime(ShmemPerfModel::_SIM_THREAD, core_id);
}

/// Get Sim Time per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CoreTimeMap getSimTimePerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, [](const core_id_t id) { return getSimTime(id); });
}

CoreTimeMap getSimTimePerCore(AllCoreIds auto... core_ids)
{
   return getSimTimePerCore({ core_ids... });
}

/// Get User Time
inline SubsecondTime getUserTime(const std::optional<core_id_t> core_id = std::nullopt)
{
   return detail::getElapsedTime(ShmemPerfModel::_USER_THREAD, core_id);
}

/// Get User Time per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CoreTimeMap getUserTimePerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, [](const core_id_t id) { return getUserTime(id); });
}

CoreTimeMap getUserTimePerCore(AllCoreIds auto... core_ids)
{
   return getUserTimePerCore({ core_ids... });
}

/// Get Total Instructions
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
UInt64 getTotalInstructions(CoreIds&& core_ids)
{
   return Sim()->getCoreManager()->getInstructionCount(core_ids);
}

UInt64 getTotalInstructions(AllCoreIds auto... core_ids)
{
   return getTotalInstructions({ core_ids... });
}

/// Get Instructions per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
InstructionCountMap getInstructionsPerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, [](const core_id_t id) { return getTotalInstructions(id); });
}

InstructionCountMap getInstructionsPerCore(AllCoreIds auto... core_ids)
{
   return getInstructionsPerCore({ core_ids... });
}

/// Get Instructions Summary
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
InstructionCountSummary getInstructionsSummary(CoreIds&& core_ids)
{
   const auto instr_per_core = getInstructionsPerCore(core_ids);
   UInt64 total_instr        = std::accumulate(instr_per_core.begin(), instr_per_core.end(), 0,
                                               [](UInt64 sum, const auto& e) { return sum + e.second; });
   return { total_instr, instr_per_core };
}

InstructionCountSummary getInstructionsSummary(AllCoreIds auto... core_ids)
{
   return getInstructionsSummary({ core_ids... });
}

/// Get Cycles
inline UInt64 getCycles(const std::optional<core_id_t> core_id = std::nullopt)
{
   return SubsecondTime::divideRounded(Sim()->getClockSkewMinimizationServer()->getGlobalTime(),
                                       detail::resolveCore(core_id)->getDvfsDomain()->getPeriod());
}

/// Get Cycles per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CyclesCountMap getCyclesPerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, getCycles);
}

CyclesCountMap getCyclesPerCore(AllCoreIds auto... core_ids)
{
   return getCyclesPerCore({ core_ids... });
}

/// Get Program Counter
inline IntPtr getProgramCounter(const std::optional<core_id_t> core_id = std::nullopt)
{
   return detail::resolveCore(core_id)->getProgramCounter();
}

/// Get Program Counter per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
ProgramCounterCoreMap getProgramCounterPerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, getProgramCounter);
}

ProgramCounterCoreMap getProgramCounterPerCore(AllCoreIds auto... core_ids)
{
   return getProgramCounterPerCore({ core_ids... });
}

/// Get Last PC to DCache
inline IntPtr getLastPCtoDCache(const std::optional<core_id_t> core_id = std::nullopt)
{
   return detail::resolveCore(core_id)->getLastPCToDCache();
}

/// Get Last PC to DCache per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
ProgramCounterCoreMap getLastPCtoDCachePerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, getLastPCtoDCache);
}

ProgramCounterCoreMap getLastPCtoDCachePerCore(AllCoreIds auto... core_ids)
{
   return getLastPCtoDCachePerCore({ core_ids... });
}

/// Get Epoch ID
inline UInt64 getEpochId(const std::optional<core_id_t> core_id = std::nullopt)
{
   auto* const metric = Sim()->getStatsManager()->getMetricObject("epoch", detail::resolveCore(core_id)->getId(), "id");
   return metric ? metric->recordMetric() : 0;
}

/// Get Epoch ID per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
EpochIdCoreMap getEpochIdPerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, getEpochId);
}

EpochIdCoreMap getEpochIdPerCore(AllCoreIds auto... core_ids)
{
   return getEpochIdPerCore({ core_ids... });
}

/// Get Core Info
inline CoreInfo getCoreInfo(const std::optional<core_id_t> core_id = std::nullopt)
{
   auto* core = detail::resolveCore(core_id);
   return {
      .id           = core->getId(),
      .pc           = core->getProgramCounter(),
      .pc_dcache    = core->getLastPCToDCache(),
      .instructions = core->getInstructionCount(),
      .cycles       = getCycles(core->getId()),
      .epoch_id     = getEpochId(core->getId())
   };
}

/// Get Core Info per Core
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CoreInfoMap getCoreInfoPerCore(CoreIds&& core_ids)
{
   return mapCoreIdsTo(core_ids, getCoreInfo);
}

CoreInfoMap getCoreInfoPerCore(AllCoreIds auto... core_ids)
{
   return getCoreInfoPerCore({ core_ids... });
}

/// Get Core Info Summary
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CoreInfoSummary getCoreInfoSummary(CoreIds&& core_ids)
{
   const auto core_infos = getCoreInfoPerCore(core_ids);
   UInt64 total_instr    = std::accumulate(core_infos.begin(), core_infos.end(), 0,
                                           [](UInt64 sum, const auto& e) {
                                           return sum + e.second.instructions;
                                        });
   return { total_instr, core_infos };
}

CoreInfoSummary getCoreInfoSummary(AllCoreIds auto... core_ids)
{
   return getCoreInfoSummary({ core_ids... });
}

/// Get Core Set Info
template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
CoreSetInfo getCoreSetInfo(CoreIds&& core_ids)
{
   return CoreSetInfo(std::views::values(getCoreInfoPerCore(core_ids)));
}

CoreSetInfo getCoreSetInfo(AllCoreIds auto... core_ids)
{
   return getCoreSetInfo({ core_ids... });
}

/// Get DRAM Accesses
inline UInt64 getDramAccesses(const DramCntlrInterface::access_t access_type)
{
   return detail::getMemAccess(access_type, "dram");
}

inline MemAccessCountMap getDramAccesses()
{
   return detail::getMemAccess("dram");
}

/// Get DRAM Access Summary
inline MemAccessCountSummary getDramAccessSummary()
{
   return detail::getMemAccessSummary("dram");
}

/// Get DRAM Info
inline DramInfo getDramInfo()
{
   return detail::summaryToDramInfo(getDramAccessSummary());
}

/// Get NVM Accesses
inline UInt64 getNvmAccesses(const DramCntlrInterface::access_t access_type)
{
   return detail::getMemAccess(access_type, "nvm");
}

inline MemAccessCountMap getNvmAccesses()
{
   return detail::getMemAccess("nvm");
}

/// Get NVM Summary
inline MemAccessCountSummary getNvmAccessSummary()
{
   return detail::getMemAccessSummary("nvm");
}

/// Get NVM Log Flushes
inline UInt64 getNvmLogFlushes()
{
   UInt64 result = 0;
   for (UInt32 i = 0; i < Sim()->getConfig()->getTotalCores(); i++)
   {
      auto* const metric = Sim()->getStatsManager()->getMetricObject("nvm", i, "log_flushes");
      result += metric ? metric->recordMetric() : 0;
   }
   return result;
}

/// Get NVM Info
inline NvmInfo getNvmInfo()
{
   return detail::summaryToNvmInfo(getNvmAccessSummary(), getNvmLogFlushes());
}

/// Get Memory Accesses
inline UInt64 getMemAccess(const DramCntlrInterface::access_t access_type)
{
   return detail::getMemAccess(access_type, "dram", "nvm");
}

inline MemAccessCountMap getMemAccess()
{
   return detail::getMemAccess("dram", "nvm");
}

/// Get Memory Access Summary
inline MemAccessCountSummary getMemAccessSummary()
{
   return detail::getMemAccessSummary("dram", "nvm");
}

/// Get Memory Info
inline MemInfo getMemInfo()
{
   const auto dram_summary           = getDramAccessSummary();
   const auto [dram_total, dram_map] = dram_summary;

   const auto nvm_summary          = getNvmAccessSummary();
   const auto [nvm_total, nvm_map] = nvm_summary;

   return {
      .reads    = dram_map.at(DramCntlrInterface::access_t::READ) + nvm_map.at(DramCntlrInterface::access_t::READ),
      .writes   = dram_map.at(DramCntlrInterface::access_t::WRITE) + nvm_map.at(DramCntlrInterface::access_t::WRITE),
      .accesses = dram_total + nvm_total,
      .dram     = detail::summaryToDramInfo(dram_summary),
      .nvm      = detail::summaryToNvmInfo(nvm_summary, getNvmLogFlushes())
   };
}

}// namespace stats