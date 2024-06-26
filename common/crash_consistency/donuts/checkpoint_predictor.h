#ifndef CHECKPOINT_PREDICTOR_H
#define CHECKPOINT_PREDICTOR_H

#include "subsecond_time.h"
#include <random>
#include <queue>

class CheckpointPredictor
{
public:

   bool predict(IntPtr address);
   void predictNext(IntPtr address, bool is_low, SubsecondTime total_time, UInt32 size);

   void insert(IntPtr address);
   void remove(IntPtr address);

protected:
   std::queue<IntPtr> m_map;

   bool canLoRtoLoW();
   bool canLoWtoLoR();
};

#endif // CHECKPOINT_PREDICTOR_H
