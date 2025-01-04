#pragma once

class CheckpointEvent {
public:
   enum class Reason
   {
      PERIODIC_TIME,
      PERIODIC_INSTRUCTIONS,
      CACHE_SET_THRESHOLD,
      CACHE_THRESHOLD
   };

   explicit CheckpointEvent(const Reason reason) : m_reason(reason) {}
   
private:
   const Reason m_reason;
};

using CheckpointReason = CheckpointEvent::Reason;