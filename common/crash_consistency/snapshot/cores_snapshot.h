#pragma once

#include "snapshot_base.h"
#include "system_stats.h"

#include <algorithm>

template<typename T>
   requires std::derived_from<T, stats::CoreInfoContainer<stats::CoreInfo>> ||
            std::derived_from<T, stats::CoreInfoContainer<stats::CoreIntervalInfo>>
class CoresSnapshotBase : public SerializableSnapshot
{
public:
   /// Get Core IDs
   core_id_t getMasterCoreId() const noexcept { return m_master_core_id; }

   /// Get Core IDs
   auto getAllIds() const noexcept { return m_core_infos.getIds(); }

   /// Get Instructions of Master Core
   UInt64 getMasterCoreInstructions() const
   {
      return getTotalInstructions(m_master_core_id);
   }

   /// Get Total Instructions
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   UInt64 getTotalInstructions(CoreIds&& core_ids) const
   {
      if (std::ranges::empty(core_ids))
         return m_core_infos.getTotalInstructions();

      return std::accumulate(core_ids.begin(), core_ids.end(), 0UL,
                             [this](auto sum, auto core_id) {
                                return sum + getInfo(core_id).instructions;
                             });
   }
   UInt64 getTotalInstructions(AllCoreIds auto... core_ids) const
   {
      return getTotalInstructions({ core_ids... });
   }

   /// Get Instructions per Core
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getInstructionsPerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getTotalInstructions(id); });
   }
   auto getInstructionsPerCore(AllCoreIds auto... core_ids) const
   {
      return getInstructionsPerCore({ core_ids... });
   }

   /// Get Cycles of Master Core
   UInt64 getMasterCoreCycles() const
   {
      return getCycles(m_master_core_id);
   }

   /// Get Cycles
   UInt64 getCycles(const core_id_t core_id) const
   {
      return getInfo(core_id).cycles;
   }

   /// Get Cycles per Core
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getCyclesPerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getCycles(id); });
   }
   auto getCyclesPerCore(AllCoreIds auto... core_ids) const
   {
      return getCyclesPerCore({ core_ids... });
   }

protected:
   const core_id_t m_master_core_id;
   const T m_core_infos;

   CoresSnapshotBase(const core_id_t master_core_id, const T& core_infos) :
       m_master_core_id(master_core_id), m_core_infos(core_infos) {}

   auto getInfo(const std::optional<core_id_t> core_id = std::nullopt) const
   {
      const auto id   = core_id.value_or(m_master_core_id);
      const auto info = m_core_infos.find(id);

      LOG_ASSERT_ERROR(info, "Core id [%d] not found in the snapshot", id);

      return *info;
   }
};

class CoresIntervalSnapshot final : public CoresSnapshotBase<stats::CoreSetIntervalInfo>, public IntervalSnapshot
{
public:
   /// Get Elapsed Epochs of Master Core
   UInt64 getMasterCoreElapsedEpochs() const
   {
      return getInfo(m_master_core_id).epochs;
   }

   /// Get Elapsed Epochs
   UInt64 getElapsedEpochs(const core_id_t core_id) const
   {
      return getInfo(core_id).epochs;
   }

   /// Get Elapsed Epochs per Core
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getElapsedEpochsPerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getElapsedEpochs(id); });
   }
   auto getElapsedEpochsPerCore(AllCoreIds auto... core_ids) const
   {
      return getEpochIds({ core_ids... });
   }

   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

private:
   CoresIntervalSnapshot(const core_id_t master_core_id,
                         const stats::CoreSetIntervalInfo& core_infos,
                         const SubsecondTime& duration) :
       CoresSnapshotBase(master_core_id, core_infos),
       IntervalSnapshot(duration) {}

   friend class CoresSnapshot;
   friend class SystemIntervalSnapshot;
};

class CoresSnapshot final : public CoresSnapshotBase<stats::CoreSetInfo>, public Snapshot
{
public:
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   static CoresSnapshot capture(CoreIds&& core_ids)
   {
      const auto master_core_id = Sim()->getCoreManager()->getCurrentCoreID();
      const auto core_infos     = stats::getCoreSetInfo(core_ids);

      LOG_ASSERT_ERROR(std::ranges::contains(core_infos.getCoreIds(), master_core_id),
                       "Master core ID [%d] is missing from the core set", master_core_id);

      return {
         master_core_id,
         core_infos,
         stats::getGlobalTime()
      };
   }

   static CoresSnapshot capture(AllCoreIds auto... core_ids)
   {
      return capture({ core_ids... });
   }

   CoresIntervalSnapshot operator-(const CoresSnapshot& rhs) const
   {
      return {
         m_master_core_id,
         m_core_infos - rhs.m_core_infos,
         m_global_time - rhs.m_global_time
      };
   }

   /// Get Program Counter of Master Core
   IntPtr getMasterCorePC() const
   {
      return getProgramCounter(m_master_core_id);
   }

   /// Get Program Counter
   IntPtr getProgramCounter(const core_id_t core_id) const
   {
      return getInfo(core_id).pc;
   }

   /// Get Program Counter per Core
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getProgramCounterPerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getProgramCounter(id); });
   }
   auto getProgramCounterPerCore(AllCoreIds auto... core_ids) const
   {
      return getProgramCounterPerCore({ core_ids... });
   }

   /// Get Last PC to DCache of Master Core
   IntPtr getMasterCorePCtoDCache() const
   {
      return getLastPCtoDCache(m_master_core_id);
   }

   /// Get Last PC to DCache
   IntPtr getLastPCtoDCache(const core_id_t core_id) const
   {
      return getInfo(core_id).pc_dcache;
   }

   /// Get Last PC to DCache per Core
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getLastPCtoDCachePerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getLastPCtoDCache(id); });
   }
   auto getLastPCtoDCachePerCore(AllCoreIds auto... core_ids) const
   {
      return getLastPCstoDCache({ core_ids... });
   }

   /// Get Epoch ID of Master Core
   UInt64 getMasterCoreEpochId() const
   {
      return getEpochId(m_master_core_id);
   }

   /// Get Epoch ID
   UInt64 getEpochId(const core_id_t core_id) const
   {
      return getInfo(core_id).epoch_id;
   }

   /// Get Epoch IDs
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   auto getEpochIdPerCore(CoreIds&& core_ids) const
   {
      return mapCoreIdsTo(core_ids, [this](const core_id_t id) { return getEpochId(id); });
   }
   auto getEpochIdPerCore(AllCoreIds auto... core_ids) const
   {
      return getEpochIds({ core_ids... });
   }

   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

private:
   CoresSnapshot(const core_id_t master_core_id,
                 const stats::CoreSetInfo& core_infos,
                 const SubsecondTime& global_time) :
       CoresSnapshotBase(master_core_id, core_infos),
       Snapshot(global_time) {}

   friend class SystemSnapshot;
};