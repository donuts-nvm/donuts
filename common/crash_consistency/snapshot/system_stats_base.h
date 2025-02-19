#pragma once

#include "concept_utils.h"
#include "dram_cntlr_interface.h"

#include <map>
#include <ranges>
#include <vector>

namespace stats::detail
{

constexpr UInt64 saturating_sub(const UInt64 lhs, const UInt64 rhs) noexcept
{
   return lhs >= rhs ? lhs - rhs : UINT64_MAX;
}
constexpr UInt64 saturating_add(const UInt64 lhs, const UInt64 rhs) noexcept
{
   return UINT64_MAX - lhs >= rhs ? lhs + rhs : UINT64_MAX;
}

}// namespace stats::detail

namespace stats
{

struct CoreIntervalInfo
{
   core_id_t id        = INVALID_CORE_ID;
   UInt64 instructions = 0;
   UInt64 cycles       = 0;
   UInt64 epochs       = 0;

   constexpr bool operator==(const CoreIntervalInfo&) const = default;

   constexpr CoreIntervalInfo& operator-=(const CoreIntervalInfo& rhs) noexcept
   {
      id           = id == rhs.id ? id : INVALID_CORE_ID;
      instructions = detail::saturating_sub(instructions, rhs.instructions);
      cycles       = detail::saturating_sub(cycles, rhs.cycles);
      epochs       = detail::saturating_sub(epochs, rhs.epochs);

      return *this;
   }

   constexpr CoreIntervalInfo& operator+=(const CoreIntervalInfo& rhs) noexcept
   {
      id           = id == rhs.id ? id : INVALID_CORE_ID;
      instructions = detail::saturating_add(instructions, rhs.instructions);
      cycles       = detail::saturating_add(cycles, rhs.cycles);
      epochs       = detail::saturating_add(epochs, rhs.epochs);

      return *this;
   }
};

constexpr CoreIntervalInfo operator-(CoreIntervalInfo lhs, const CoreIntervalInfo& rhs) noexcept
{
   return lhs -= rhs;
}

constexpr CoreIntervalInfo operator+(CoreIntervalInfo lhs, const CoreIntervalInfo& rhs) noexcept
{
   return lhs += rhs;
}

struct CoreInfo
{
   core_id_t id        = INVALID_CORE_ID;
   IntPtr pc           = 0;
   IntPtr pc_dcache    = 0;
   UInt64 instructions = 0;
   UInt64 cycles       = 0;
   UInt64 epoch_id     = 0;

   constexpr bool operator==(const CoreInfo&) const = default;
};

constexpr CoreIntervalInfo operator-(const CoreInfo& lhs, const CoreInfo& rhs) noexcept
{
   return {
      .id           = lhs.id == rhs.id ? lhs.id : INVALID_CORE_ID,
      .instructions = detail::saturating_sub(lhs.instructions, rhs.instructions),
      .cycles       = detail::saturating_sub(lhs.cycles, rhs.cycles),
      .epochs       = detail::saturating_sub(lhs.epoch_id, rhs.epoch_id)
   };
}

template<typename T>
   requires util::concepts::AnyOf<T, CoreInfo, CoreIntervalInfo>
class CoreInfoContainer
{
public:
   bool operator==(const CoreInfoContainer&) const = default;

   auto getCoreIds() const { return std::views::keys(m_core_infos); }

   auto getInfoPerCore() const { return std::views::values(m_core_infos); }

   std::optional<T> find(const core_id_t core_id) const
   {
      const auto it = m_core_infos.find(core_id);
      return it != m_core_infos.end() ? std::optional(it->second) : std::nullopt;
   }

   UInt64 getTotalInstructions() const { return m_total_instructions; }

protected:
   std::map<core_id_t, T> m_core_infos;
   UInt64 m_total_instructions = 0;

   template<util::concepts::RangeOf<T> R = std::initializer_list<T>>
   explicit CoreInfoContainer(R&& core_infos)
   {
      for (auto&& core_info: std::forward<R>(core_infos))
      {
         m_core_infos.emplace(core_info.id, core_info);
         m_total_instructions += core_info.instructions;
      }
   }
};

class CoreSetIntervalInfo : public CoreInfoContainer<CoreIntervalInfo>
{
public:
   template<util::concepts::RangeOf<CoreIntervalInfo> R = std::initializer_list<CoreIntervalInfo>>
   explicit CoreSetIntervalInfo(R&& core_infos) :
       CoreInfoContainer(std::forward<R>(core_infos)) {}

   explicit CoreSetIntervalInfo(util::concepts::PackOf<CoreIntervalInfo> auto&&... core_infos) :
       CoreInfoContainer({ core_infos... }) {}
};

class CoreSetInfo : public CoreInfoContainer<CoreInfo>
{
public:
   template<util::concepts::RangeOf<CoreInfo> R = std::initializer_list<CoreInfo>>
   explicit CoreSetInfo(R&& core_infos) :
       CoreInfoContainer(std::forward<R>(core_infos)) {}

   explicit CoreSetInfo(util::concepts::PackOf<CoreInfo> auto&&... core_infos) :
       CoreInfoContainer({ core_infos... }) {}

   friend constexpr CoreSetIntervalInfo operator-(const CoreSetInfo& lhs, const CoreSetInfo& rhs) noexcept;
};

constexpr CoreSetIntervalInfo operator-(const CoreSetInfo& lhs, const CoreSetInfo& rhs) noexcept
{
   std::vector<CoreIntervalInfo> result;
   for (const auto& [core_id, core_info]: lhs.m_core_infos)
   {
      const auto& rhs_core_info = rhs.find(core_id).value_or(CoreInfo{ .id = core_id });
      result.emplace_back(core_info - rhs_core_info);
   }
   return CoreSetIntervalInfo(result);
}

struct DramInfo
{
   UInt64 reads    = 0;
   UInt64 writes   = 0;
   UInt64 accesses = 0;

   constexpr bool operator==(const DramInfo&) const = default;

   constexpr DramInfo& operator-=(const DramInfo& rhs) noexcept
   {
      reads    = detail::saturating_sub(reads, rhs.reads);
      writes   = detail::saturating_sub(writes, rhs.writes);
      accesses = detail::saturating_sub(accesses, rhs.accesses);

      return *this;
   }

   constexpr DramInfo& operator+=(const DramInfo& rhs) noexcept
   {
      reads    = detail::saturating_add(reads, rhs.reads);
      writes   = detail::saturating_add(writes, rhs.writes);
      accesses = detail::saturating_add(accesses, rhs.accesses);

      return *this;
   }
};

constexpr DramInfo operator-(DramInfo lhs, const DramInfo& rhs) noexcept
{
   return lhs -= rhs;
}

constexpr DramInfo operator+(DramInfo lhs, const DramInfo& rhs) noexcept
{
   return lhs += rhs;
}

struct NvmInfo
{
   UInt64 reads       = 0;
   UInt64 writes      = 0;
   UInt64 logs        = 0;
   UInt64 accesses    = 0;
   UInt64 log_flushes = 0;

   constexpr bool operator==(const NvmInfo&) const = default;

   constexpr NvmInfo& operator-=(const NvmInfo& rhs) noexcept
   {
      reads       = detail::saturating_sub(reads, rhs.reads);
      writes      = detail::saturating_sub(writes, rhs.writes);
      logs        = detail::saturating_sub(logs, rhs.logs);
      accesses    = detail::saturating_sub(accesses, rhs.accesses);
      log_flushes = detail::saturating_sub(log_flushes, rhs.log_flushes);

      return *this;
   }

   constexpr NvmInfo& operator+=(const NvmInfo& rhs) noexcept
   {
      reads       = detail::saturating_add(reads, rhs.reads);
      writes      = detail::saturating_add(writes, rhs.writes);
      logs        = detail::saturating_add(logs, rhs.logs);
      accesses    = detail::saturating_add(accesses, rhs.accesses);
      log_flushes = detail::saturating_add(log_flushes, rhs.log_flushes);

      return *this;
   }
};

constexpr NvmInfo operator-(NvmInfo lhs, const NvmInfo& rhs) noexcept
{
   return lhs -= rhs;
}

constexpr NvmInfo operator+(NvmInfo lhs, const NvmInfo& rhs) noexcept
{
   return lhs += rhs;
}

struct MemInfo
{
   UInt64 reads    = 0;
   UInt64 writes   = 0;
   UInt64 accesses = 0;
   DramInfo dram;
   NvmInfo nvm;

   constexpr bool operator==(const MemInfo&) const = default;

   constexpr MemInfo& operator-=(const MemInfo& rhs) noexcept
   {
      reads    = detail::saturating_sub(reads, rhs.reads);
      writes   = detail::saturating_sub(writes, rhs.writes);
      accesses = detail::saturating_sub(accesses, rhs.accesses);
      dram     = dram - rhs.dram;
      nvm      = nvm - rhs.nvm;

      return *this;
   }

   constexpr MemInfo& operator+=(const MemInfo& rhs) noexcept
   {
      reads    = detail::saturating_add(reads, rhs.reads);
      writes   = detail::saturating_add(writes, rhs.writes);
      accesses = detail::saturating_add(accesses, rhs.accesses);
      dram     = dram + rhs.dram;
      nvm      = nvm + rhs.nvm;

      return *this;
   }
};

constexpr MemInfo operator-(MemInfo lhs, const MemInfo& rhs) noexcept
{
   return lhs -= rhs;
}

constexpr MemInfo operator+(MemInfo lhs, const MemInfo& rhs) noexcept
{
   return lhs += rhs;
}

}// namespace stats

using CoreTimeMap           = std::map<core_id_t, SubsecondTime>;
using InstructionCountMap   = std::map<core_id_t, UInt64>;
using CyclesCountMap        = std::map<core_id_t, IntPtr>;
using ProgramCounterCoreMap = std::map<core_id_t, IntPtr>;
using EpochIdCoreMap        = std::map<core_id_t, UInt64>;
using CoreInfoMap           = std::map<core_id_t, stats::CoreInfo>;
using MemAccessCountMap     = std::map<DramCntlrInterface::access_t, UInt64>;

using InstructionCountSummary = std::pair<UInt64, InstructionCountMap>;
using CoreInfoSummary         = std::pair<UInt64, CoreInfoMap>;
using MemAccessCountSummary   = std::pair<UInt64, MemAccessCountMap>;