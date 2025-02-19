#include "memory_snapshot.h"

String MemorySnapshotBase::memInfoJsonBody() const
{
   std::ostringstream oss;

   oss << "\"reads\":" << m_mem_info.reads << ","
       << "\"writes\":" << m_mem_info.writes << ","
       << "\"accesses\":" << m_mem_info.accesses << ","
       << "\"devices\":["
       << "{\"dram\":{"
       << "\"reads\":" << m_mem_info.dram.reads << ","
       << "\"writes\":" << m_mem_info.dram.writes << ","
       << "\"accesses\":" << m_mem_info.dram.accesses << "}},"
       << "{\"nvm\":{\n"
       << "\"reads\":" << m_mem_info.nvm.reads << ","
       << "\"writes\":" << m_mem_info.nvm.writes << ","
       << "\"logs\":" << m_mem_info.nvm.logs << ","
       << "\"accesses\":" << m_mem_info.nvm.accesses << ","
       << "\"log_flushes\":" << m_mem_info.nvm.log_flushes << "}}]";

   return oss.str().c_str();
}

String MemorySnapshotBase::memInfoYamlBody(const UInt8 tab_level) const
{
   std::ostringstream oss;

   oss << indent(tab_level) << "reads: " << m_mem_info.reads << "\n"
       << indent(tab_level) << "writes: " << m_mem_info.writes << "\n"
       << indent(tab_level) << "accesses: " << m_mem_info.accesses << "\n"
       << indent(tab_level) << "devices:\n"
       << indent(tab_level + 1) << "- dram:\n"
       << indent(tab_level + 2) << "  reads: " << m_mem_info.dram.reads << "\n"
       << indent(tab_level + 2) << "  writes: " << m_mem_info.dram.writes << "\n"
       << indent(tab_level + 2) << "  accesses: " << m_mem_info.dram.accesses << "\n"
       << indent(tab_level + 1) << "- nvm:\n"
       << indent(tab_level + 2) << "  reads: " << m_mem_info.nvm.reads << "\n"
       << indent(tab_level + 2) << "  writes: " << m_mem_info.nvm.writes << "\n"
       << indent(tab_level + 2) << "  logs: " << m_mem_info.nvm.logs << "\n"
       << indent(tab_level + 2) << "  accesses: " << m_mem_info.nvm.accesses << "\n"
       << indent(tab_level + 2) << "  log_flushes: " << m_mem_info.nvm.log_flushes;

   return oss.str().c_str();
}

String MemoryIntervalSnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{";
   if (!embed) oss << "\"duration\":" << m_duration.getNS() << ",";
   oss << memInfoJsonBody().c_str() << "}";

   return oss.str().c_str();
}

String MemoryIntervalSnapshot::toYaml(const bool embed) const
{
   const int level = embed ? 1 : 0;
   std::ostringstream oss;

   oss << indent(level) << "memory_interval_snapshot:\n";
   if (!embed) oss << indent(level + 1) << "duration: " << m_duration.getNS() << "\n";
   oss << memInfoYamlBody(level + 1).c_str();

   return oss.str().c_str();
}

String MemorySnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{";
   if (!embed) oss << "\"global_time\":" << m_global_time.getNS() << ",";
   oss << memInfoJsonBody().c_str() << "}";

   return oss.str().c_str();
}

String MemorySnapshot::toYaml(const bool embed) const
{
   const int level = embed ? 1 : 0;
   std::ostringstream oss;

   oss << indent(level) << "memory_snapshot:\n";
   if (!embed) oss << indent(level + 1) << "global_time: " << m_global_time.getNS() << "\n";
   oss << memInfoYamlBody(level + 1).c_str();

   return oss.str().c_str();
}
