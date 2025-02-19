#pragma once

#include "cores_snapshot.h"
#include "memory_snapshot.h"

template<typename CoreSnap, typename MemSnap>
   requires(std::same_as<CoreSnap, CoresSnapshot> || std::same_as<CoreSnap, CoresIntervalSnapshot>) &&
           std::derived_from<MemSnap, MemorySnapshotBase>
class SystemSnapshotBase : public SerializableSnapshot
{
public:
   const CoreSnap& getCoresInfo() const { return m_cores_snapshot; }
   const MemSnap& getMemoryInfo() const { return m_memory_snapshot; }

protected:
   CoreSnap m_cores_snapshot;
   MemSnap m_memory_snapshot;

   explicit SystemSnapshotBase(const CoreSnap& cores_snapshot, const MemSnap& memory_snapshot) :
       m_cores_snapshot(cores_snapshot), m_memory_snapshot(memory_snapshot) {}
};

class SystemIntervalSnapshot final : public SystemSnapshotBase<CoresIntervalSnapshot, MemoryIntervalSnapshot>, public IntervalSnapshot
{
public:
   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

private:
   SystemIntervalSnapshot(const CoresIntervalSnapshot& cores_snapshot,
                          const MemoryIntervalSnapshot& memory_snapshot,
                          const SubsecondTime& duration) :
       SystemSnapshotBase(cores_snapshot, memory_snapshot),
       IntervalSnapshot(duration) {}

   friend class SystemSnapshot;
};

class SystemSnapshot final : public SystemSnapshotBase<CoresSnapshot, MemorySnapshot>, public Snapshot
{
public:
   template<CoreIdRange CoreIds = std::initializer_list<core_id_t>>
   static SystemSnapshot capture(CoreIds&& core_ids)
   {
      return {
         CoresSnapshot::capture(core_ids),
         MemorySnapshot::capture(),
         stats::getGlobalTime()
      };
   }

   static auto capture(AllCoreIds auto... core_ids)
   {
      return capture({ core_ids... });
   }

   SystemIntervalSnapshot operator-(const SystemSnapshot& rhs) const
   {
      return {
         m_cores_snapshot - rhs.m_cores_snapshot,
         m_memory_snapshot - rhs.m_memory_snapshot,
         m_global_time - rhs.m_global_time
      };
   }

   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

   static SystemSnapshot INITIAL()
   {
      const auto initial_time = SubsecondTime::Zero();
      return {
         { 0, stats::CoreSetInfo({}), initial_time },
         { {}, initial_time },
         initial_time
      };
   }

private:
   SystemSnapshot(const CoresSnapshot& cores_snapshot,
                  const MemorySnapshot& memory_snapshot,
                  const SubsecondTime& global_time) :
       SystemSnapshotBase(cores_snapshot, memory_snapshot),
       Snapshot(global_time) {}
};
