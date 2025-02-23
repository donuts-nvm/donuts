#include "project.h"
#include "config.hpp"
#include "simulator.h"
#include <algorithm>

Project::Project() : m_type(loadType()), m_name(toName(m_type)) {}

const char*
Project::toName(const ProjectType project_type)
{
   switch (project_type)
   {
      case ProjectType::BASELINE:
         return "Baseline";
      case ProjectType::DONUTS:
         return "dOnuts";
      default:
         return "Unknown";
   }
}

ProjectType
Project::toType(const String& project_name)
{
   String name = project_name;
   std::ranges::transform(name.begin(), name.end(), name.begin(), tolower);

   if (name == "baseline" || name == "default")
      return ProjectType::BASELINE;
   if (name == "donuts")
      return ProjectType::DONUTS;

   return ProjectType::UNKNOWN;
}

ProjectType
Project::loadType()
{
   const auto* key    = "general/project_type";
   const auto project = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getString(key) : "baseline";

   const auto project_type = toType(project);
   LOG_ASSERT_ERROR(project_type != ProjectType::UNKNOWN, "Unknown project: '%s'", project.c_str())

   return project_type;
}
