#pragma once

#include "log_policy.h"
#include "subsecond_time.h"
#include <optional>
#include <vector>

enum class CheckpointReason
{
   PERIODIC_TIME,
   PERIODIC_INSTRUCTIONS,
   CACHE_SET_THRESHOLD,
   CACHE_THRESHOLD
};

class SystemSnapshot
{
public:
   SystemSnapshot(const SystemSnapshot&)            = default;
   SystemSnapshot& operator=(const SystemSnapshot&) = delete;

   [[nodiscard]] SubsecondTime getGlobalTime() const { return m_global_time; }
   [[nodiscard]] UInt64 getTotalInstructions() const { return m_total_instr; }
   [[nodiscard]] UInt64 getTotalInstructionsVD() const { return m_total_instr_vd; }

   static SystemSnapshot capture(const std::vector<core_id_t>& vd_cores = {});

private:
   SystemSnapshot(const SubsecondTime& time, const UInt64 total_instr, const UInt64 total_instr_vd) :
       m_global_time(time), m_total_instr(total_instr), m_total_instr_vd(total_instr_vd) {}

   const SubsecondTime m_global_time;
   const UInt64 m_total_instr;
   const UInt64 m_total_instr_vd;
};

// class NvmSnapshot ??? (it must get nvm stats automatically)

class NvmStats
{
public:
   void plusRead() { m_num_reads++; }
   void plusWrite() { m_num_writes++; }
   void plusLog(const UInt64 log_size)
   {
      m_num_logs++;
      m_log_size += log_size;
   }

   [[nodiscard]] UInt64 getNumReads() const { return m_num_reads; }
   [[nodiscard]] UInt64 getNumWrites() const { return m_num_writes; }
   [[nodiscard]] UInt64 getNumLogs() const { return m_num_logs; }
   [[nodiscard]] UInt64 getLogSize() const { return m_log_size; }

private:
   UInt64 m_num_reads{ 0 };
   UInt64 m_num_writes{ 0 };
   UInt64 m_num_logs{ 0 };
   UInt64 m_log_size{ 0 };
};

class EpochInfo
{
public:
   EpochInfo(const EpochInfo&)            = default;
   EpochInfo& operator=(const EpochInfo&) = delete;

   static EpochInfo start(UInt64 eid, IntPtr pc, const SystemSnapshot& snapshot);
   void finalize(CheckpointReason reason, const SystemSnapshot& snapshot);
   void persistedIn(const SubsecondTime& time);

   [[nodiscard]] UInt64 getEpochID() const { return m_eid; }
   [[nodiscard]] IntPtr getProgramCounter() const { return m_pc; }
   [[nodiscard]] SubsecondTime getGlobalTime() const { return m_snapshot.getGlobalTime(); }
   [[nodiscard]] UInt64 getTotalInstructions() const { return m_snapshot.getTotalInstructions(); }
   [[nodiscard]] UInt64 getTotalInstructionsVD() const { return m_snapshot.getTotalInstructionsVD(); }
   [[nodiscard]] CheckpointReason getReason() const { return m_reason; }
   [[nodiscard]] SubsecondTime getDuration() const { return m_duration; }
   [[nodiscard]] UInt64 getNumInstructions() const { return m_num_instr; }
   [[nodiscard]] UInt64 getNumReads() const { return m_nvm_stats.getNumReads(); }
   [[nodiscard]] UInt64 getNumWrites() const { return m_nvm_stats.getNumWrites(); }
   [[nodiscard]] UInt64 getNumLogs() const { return m_nvm_stats.getNumLogs(); }
   [[nodiscard]] LoggingPolicy getLogPolicy() const { return m_log_policy; }
   [[nodiscard]] UInt64 getCheckpointSize() const { return m_nvm_stats.getLogSize(); }
   [[nodiscard]] SubsecondTime getPersistenceTime() const { return m_persistence_time; }
   [[nodiscard]] const NvmStats& getNvmStats() const { return m_nvm_stats; }
   [[nodiscard]] std::optional<double> getWriteAmplification() const
   {
      const auto writes = m_nvm_stats.getNumWrites();
      const auto logs   = m_nvm_stats.getNumLogs();

      if (writes == 0 || logs == 0) return std::nullopt;
      return static_cast<double>(logs) / static_cast<double>(writes);
   }

   void print() const;

private:
   explicit EpochInfo(UInt64 eid, IntPtr pc, const SystemSnapshot& snapshot);

   const UInt64 m_eid;
   const IntPtr m_pc;
   const SystemSnapshot m_snapshot;
   CheckpointReason m_reason;
   SubsecondTime m_duration;
   UInt64 m_num_instr;
   NvmStats m_nvm_stats;
   LoggingPolicy m_log_policy;
   SubsecondTime m_persistence_time;
};
