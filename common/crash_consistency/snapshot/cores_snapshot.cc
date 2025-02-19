#include "cores_snapshot.h"

String CoresIntervalSnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{";
   if (!embed) oss << "duration:" << m_duration.getNS() << ",";
   oss << "\"cores\":[";
   const auto& infos = m_core_infos.getInfoPerCore();
   for (auto it = infos.begin(); it != infos.end(); ++it)
   {
      const auto& [id, instr, cycles, epochs] = *it;
      oss << "{\"id\": " << id << ","
          << "\"instructions\": " << instr << ","
          << "\"cycles\": " << cycles << ","
          << "\"epochs\": " << epochs << "}";
      if (std::next(it) != infos.end()) oss << ",";
   }
   oss << "],\"total_instructions\": " << m_core_infos.getTotalInstructions() << "}";

   return oss.str().c_str();
}

String CoresIntervalSnapshot::toYaml(const bool embed) const
{
   const int base = embed ? 1 : 0;
   std::ostringstream oss;

   oss << indent(base + 0) << "cores_interval_snapshot:\n";
   if (!embed) oss << indent(base + 1) << "duration: " << m_duration.getNS() << "\n";
   oss << indent(base + 1) << "cores:\n";
   for (const auto& [id, instr, cycles, epochs]: m_core_infos.getInfoPerCore())
   {
      oss << indent(base + 2) << "- id: " << id << "\n"
          << indent(base + 2) << "  instructions: " << instr << "\n"
          << indent(base + 2) << "  cycles: " << cycles << "\n"
          << indent(base + 2) << "  epochs: " << epochs << "\n";
   }
   oss << indent(base + 1) << "total_instructions: " << m_core_infos.getTotalInstructions();

   return oss.str().c_str();
}

String CoresSnapshot::toJson(const bool embed) const
{
   std::ostringstream oss;

   oss << "{";
   if (!embed) oss << "global_time:" << m_global_time.getNS() << ",";
   oss << "\"cores\":[";
   const auto& infos = m_core_infos.getInfoPerCore();
   for (auto it = infos.begin(); it != infos.end(); ++it)
   {
      const auto& [id, pc, pc_dcache, instr, cycles, eid] = *it;
      oss << "{\"id\": " << id << ","
          << "\"pc\":\"0x" << std::hex << pc << "\","
          << "\"pc_dcache\":\"0x" << pc_dcache << std::dec << "\","
          << "\"instructions\": " << instr << ","
          << "\"cycles\": " << cycles << ","
          << "\"epoch_id\": " << eid << "}";
      if (std::next(it) != infos.end()) oss << ",";
   }
   oss << "],\"total_instructions\": " << m_core_infos.getTotalInstructions() << "}";

   return oss.str().c_str();
}

String CoresSnapshot::toYaml(const bool embed) const
{
   const int base = embed ? 1 : 0;
   std::ostringstream oss;

   oss << indent(base + 0) << "cores_snapshot:\n";
   if (!embed) oss << indent(base + 1) << "global_time: " << m_global_time.getNS() << "\n";
   oss << indent(base + 1) << "cores:\n";
   for (const auto& [id, pc, pc_dcache, instr, cycles, eid] : m_core_infos.getInfoPerCore())
   {
      oss << indent(base + 2) << "- id: " << id << "\n"
          << indent(base + 2) << "  pc: 0x" << std::hex << pc << "\n"
          << indent(base + 2) << "  pc_dcache: 0x" << pc_dcache << std::dec << "\n"
          << indent(base + 2) << "  instructions: " << instr << "\n"
          << indent(base + 2) << "  cycles: " << cycles << "\n"
          << indent(base + 2) << "  epoch_id: " << eid << "\n";
   }
   oss << indent(base + 1) << "total_instructions: " << m_core_infos.getTotalInstructions();

   return oss.str().c_str();
}