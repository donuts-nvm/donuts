#include "log_policy.h"
#include <algorithm>
#include <log.h>

constexpr const char*
LogPolicy::getName(const Type log_policy)
{
   switch (log_policy)
   {
      case Type::LOGGING_DISABLED:
         return "Disabled";
      case Type::LOGGING_ON_READ:
         return "Logging on Read";
      case Type::LOGGING_ON_WRITE:
         return "Logging on Write";
      case Type::LOGGING_HYBRID:
         return "Logging Hybrid";
      case Type::LOGGING_ON_COMMAND:
         return "Logging on Command";
   }
   LOG_PRINT_ERROR("Invalid log policy: '%i'", log_policy);
}

constexpr const char*
LogPolicy::getAbbrName(const Type log_policy)
{
   switch (log_policy)
   {
      case Type::LOGGING_DISABLED:
         return "Disabled";
      case Type::LOGGING_ON_READ:
         return "LoR";
      case Type::LOGGING_ON_WRITE:
         return "LoW";
      case Type::LOGGING_HYBRID:
         return "Hybrid";
      case Type::LOGGING_ON_COMMAND:
         return "LoC";
   }
   LOG_PRINT_ERROR("Invalid log policy: '%i'", log_policy);
}

LogPolicy::Type
LogPolicy::fromType(const String& log_policy)
{
   String policy = log_policy;
   std::ranges::transform(policy.begin(), policy.end(), policy.begin(), tolower);

   if (policy == "logging_disabled" || policy == "disabled") return Type::LOGGING_DISABLED;
   if (policy == "logging_on_read" || policy == "lor") return Type::LOGGING_ON_READ;
   if (policy == "logging_on_write" || policy == "low") return Type::LOGGING_ON_WRITE;
   if (policy == "logging_hybrid" || policy == "hybrid") return Type::LOGGING_HYBRID;
   if (policy == "logging_on_command" || policy == "loc" || policy == "cmd") return Type::LOGGING_ON_COMMAND;

   LOG_PRINT_ERROR("Invalid log policy: '%s'", log_policy.c_str());
}
