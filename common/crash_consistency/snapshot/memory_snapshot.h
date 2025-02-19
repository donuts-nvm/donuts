#pragma once

#include "snapshot_base.h"
#include "system_stats.h"

class MemorySnapshotBase : public SerializableSnapshot
{
public:
   /// DRAM Statistics
   UInt64 getDramReads() const { return m_mem_info.dram.reads; }
   UInt64 getDramWrites() const { return m_mem_info.dram.writes; }
   UInt64 getDramAccesses() const { return m_mem_info.dram.accesses; }

   /// NVM Statistics
   UInt64 getNvmReads() const { return m_mem_info.nvm.reads; }
   UInt64 getNvmWrites() const { return m_mem_info.nvm.writes; }
   UInt64 getNvmLogs() const { return m_mem_info.nvm.logs; }
   UInt64 getNvmLogFlushes() const { return m_mem_info.nvm.log_flushes; }
   UInt64 getNvmAccesses() const { return m_mem_info.nvm.accesses; }

   /// Memory (DRAM + NVM) Statistics
   UInt64 getMemReads() const { return m_mem_info.reads; }
   UInt64 getMemWrites() const { return m_mem_info.writes; }
   UInt64 getMemAccesses() const { return m_mem_info.accesses; }

protected:
   const stats::MemInfo m_mem_info;

   explicit MemorySnapshotBase(const stats::MemInfo& mem_info) :
       m_mem_info(mem_info) {}

   String memInfoJsonBody() const;
   String memInfoYamlBody(UInt8 tab_level) const;
};

class MemorySnapshot;

class MemoryIntervalSnapshot final : public MemorySnapshotBase, public IntervalSnapshot
{
protected:
   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

private:
   MemoryIntervalSnapshot(const stats::MemInfo& mem_info,
                          const SubsecondTime& duration) :
       MemorySnapshotBase(mem_info),
       IntervalSnapshot(duration) {}

   friend class MemorySnapshot;
   friend class SystemIntervalSnapshot;

   friend constexpr MemoryIntervalSnapshot operator-(const MemorySnapshot& lhs, const MemorySnapshot& rhs) noexcept;
};

class MemorySnapshot final : public MemorySnapshotBase, public Snapshot
{
public:
   static MemorySnapshot capture()
   {
      return {
         stats::getMemInfo(),
         stats::getGlobalTime()
      };
   }

   friend constexpr MemoryIntervalSnapshot operator-(const MemorySnapshot& lhs, const MemorySnapshot& rhs) noexcept;

protected:
   String toJson(bool embed) const override;
   String toYaml(bool embed) const override;

private:
   MemorySnapshot(const stats::MemInfo& mem_info,
                  const SubsecondTime& global_time) :
       MemorySnapshotBase(mem_info),
       Snapshot(global_time) {}

   friend class SystemSnapshot;
};

constexpr MemoryIntervalSnapshot operator-(const MemorySnapshot& lhs, const MemorySnapshot& rhs) noexcept
{
   return {
      lhs.m_mem_info - rhs.m_mem_info,
      lhs.m_global_time - rhs.m_global_time
   };
}