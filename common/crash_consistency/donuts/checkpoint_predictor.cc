#include "checkpoint_predictor.h"

bool CheckpointPredictor::predict(IntPtr address)
{
   std::random_device r;
   std::default_random_engine e1(r());
   std::uniform_int_distribution<int> uniform_dist(1, 100);

   return uniform_dist(e1) > 50;
}

void
CheckpointPredictor::predictNext(IntPtr pc, bool is_low, SubsecondTime total_time, UInt32 size)
{
   if (!is_low && canLoRtoLoW())
      remove(pc);
   else if (is_low && canLoWtoLoR())
      insert(pc);
}

void
CheckpointPredictor::insert(IntPtr address)
{
   m_map.emplace(address);
}

void
CheckpointPredictor::remove(IntPtr address)
{

}

bool
CheckpointPredictor::canLoRtoLoW()
{
   return predict(1000);
}

bool
CheckpointPredictor::canLoWtoLoR()
{
   return predict(1000);
}
