#include "system_snapshot.h"

String SystemIntervalSnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{\"duration\":" << m_duration.getNS() << ","
       << "\"cores_interval_snapshot\":" << m_cores_snapshot.toJson(true) << ","
       << "\"memory_interval_snapshot\":" << m_memory_snapshot.toJson(true) << "}";

   return oss.str().c_str();
}

String SystemIntervalSnapshot::toYaml(const bool embed) const
{
   std::ostringstream oss;

   oss << "system_interval_snapshot:\n"
       << indent(1) << "duration: " << m_duration.getNS() << "\n"
       << m_cores_snapshot.toYaml(true) << "\n"
       << m_memory_snapshot.toYaml(true);

   return oss.str().c_str();
}

String SystemSnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{\"global_time\":" << m_global_time.getNS() << ","
       << "\"cores_snapshot\":" << m_cores_snapshot.toJson(true) << ","
       << "\"memory_snapshot\":" << m_memory_snapshot.toJson(true) << "}";

   return oss.str().c_str();
}

String SystemSnapshot::toYaml(const bool embed) const
{
   std::ostringstream oss;

   oss << "system_snapshot:\n"
       << indent(1) << "global_time: " << m_global_time.getNS() << "\n"
       << m_cores_snapshot.toYaml(true) << "\n"
       << m_memory_snapshot.toYaml(true);

   return oss.str().c_str();
}
