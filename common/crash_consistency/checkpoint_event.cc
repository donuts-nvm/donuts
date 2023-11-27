#include "checkpoint_event.h"

const char *CheckpointEvent::TypeString(CheckpointEvent::Type type)
{
   switch (type)
   {
      case PERIODIC_TIME:           return "PERIODIC_TIME";
      case PERIODIC_INSTRUCTIONS:   return "PERIODIC_INSTRUCTIONS";
      case CACHE_SET_THRESHOLD:     return "CACHE_SET_THRESHOLD";
      case CACHE_THRESHOLD:         return "CACHE_THRESHOLD";
      default:                      return "?";
   }
}