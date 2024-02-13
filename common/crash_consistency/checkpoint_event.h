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

   // o que motivou o checkpoint
   // tempo em que o checkpoint foi realizado
   // instrução em que o checkpoint foi realizado
   // quantidade de logs desse checkpoint
   // -- o tamanho do checkpoint
   // -- o tempo de persistência desse checkpoint

   static const char *TypeString(Type type);
};

#endif // CHECKPOINT_EVENT_H
