#ifndef CHECKPOINT_EVENT_H
#define CHECKPOINT_EVENT_H

class CheckpointEvent
{
public:
   typedef enum type_t
   {
      PERIODIC_TIME,
      PERIODIC_INSTRUCTIONS,
      CACHE_SET_THRESHOLD,
      CACHE_THRESHOLD,
      NUM_EVENT_TYPES
   } Type;

   static const char *TypeString(Type type);
};

#endif // CHECKPOINT_EVENT_H
