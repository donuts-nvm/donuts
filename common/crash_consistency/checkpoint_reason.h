#pragma once

enum class CheckpointReason
{
   PERIODIC_TIME,
   PERIODIC_INSTRUCTIONS,
   CACHE_SET_THRESHOLD,
   CACHE_THRESHOLD
};

constexpr const char*
toString(const CheckpointReason reason)
{
   switch (reason)
   {
      case CheckpointReason::PERIODIC_TIME: return "Periodic Time";
      case CheckpointReason::PERIODIC_INSTRUCTIONS: return "Periodic Instructions";
      case CheckpointReason::CACHE_SET_THRESHOLD: return "Cache Set Threshold";
      case CheckpointReason::CACHE_THRESHOLD: return "Cache Threshold";

      default: return "Unknown";
   }
}
