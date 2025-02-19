#pragma once

#include "fixed_types.h"

class LogPolicy
{
public:
   enum class Type
   {
      LOGGING_DISABLED = 0,
      LOGGING_ON_READ,
      LOGGING_ON_WRITE,
      LOGGING_HYBRID,
      LOGGING_ON_COMMAND
   };

   static Type fromType(const String& log_policy);
   static constexpr const char* getName(Type log_policy);
   static constexpr const char* getAbbrName(Type log_policy);
};

using LoggingPolicy = LogPolicy::Type;
