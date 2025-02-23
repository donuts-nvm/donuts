#pragma once

#include "fixed_types.h"

class Project
{
public:
   enum class Type
   {
      UNKNOWN,
      BASELINE,
      DONUTS
   };

   Project();

   [[nodiscard]] Type getType() const { return m_type; }
   [[nodiscard]] const char* getName() const { return m_name.c_str(); }

private:
   const Type m_type;
   const String m_name;

   static Type loadType();

   static const char* toName(Type project_type);
   static Type toType(const String& project_name);
};

using ProjectType = Project::Type;
