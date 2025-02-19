#include "epoch_info.h"

EpochInfo::EpochInfo(const SystemSnapshot& initial_snapshot,
                     const SystemSnapshot& final_snapshot,
                     const CheckpointReason reason,
                     const LoggingPolicy logging_policy) :
    m_initial_snapshot(initial_snapshot),
    m_final_snapshot(final_snapshot),
    // m_delta(SystemSnapshot::delta(initial_snapshot, final_snapshot)),
    m_reason(reason),
    m_logging_policy(logging_policy)
{
   // printf("delta_%s\n", m_delta.toString().c_str());
}