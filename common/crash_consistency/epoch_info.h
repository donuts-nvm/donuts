#pragma once

#include "log_policy.h"
#include "system_snapshot.h"

class EpochInfo
{
public:
   EpochInfo(const SystemSnapshot& initial_snapshot, const SystemSnapshot& final_snapshot, CheckpointReason reason,
             LoggingPolicy logging_policy = LoggingPolicy::LOGGING_DISABLED);

private:
   // const UInt64 m_eid;
   const SystemSnapshot m_initial_snapshot;
   const SystemSnapshot m_final_snapshot;
   // const ExecutionIntervalSnapshot m_delta;
   const CheckpointReason m_reason;
   const LoggingPolicy m_logging_policy;

   // const IntPtr m_pc;
   // const SubsecondTime m_global_time;
   // const UInt64 m_total_instr;
   // const UInt64 m_total_instr_vd;
   // const CheckpointReason m_reason;
   // const SubsecondTime m_duration;
   // const UInt64 m_num_instr;
   // const SystemStats::MemInfo m_mem_info;
   // const LoggingPolicy m_log_policy;
   // const SubsecondTime m_persistence_time;

   // const CoreSet m_cores;
   // const SystemStats::CoreSetInfo m_core_set_info;
   // const SystemStats::MemInfo m_mem_info;
   // const SystemStats::GlobalInfo m_global_info;

   // const std::vector<core_id_t> cores;
   // const UInt64 total_instructions; // current
   // const UInt64 total_instructions_vd; // all cores
   // const std::unordered_map<core_id_t, UInt64> total_instructions_per_core;
   // const std::unordered_map<core_id_t, UInt64> epoch_id_per_core_vd;
   // const SystemStats::MemInfo mem_stats;
   // const SystemStats::GlobalInfo global_stats;
};

class EpochInfoBuilder
{
public:
   EpochInfoBuilder& start(const SystemSnapshot& initial_snapshot);
   EpochInfoBuilder& end(CheckpointReason reason, const SystemSnapshot& final_snapshot);
   EpochInfoBuilder& loggingPolicy(LoggingPolicy logging_policy);
   EpochInfoBuilder& persistedIn(const SubsecondTime& time);
   EpochInfo build();
};
