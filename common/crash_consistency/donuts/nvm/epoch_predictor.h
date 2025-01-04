#pragma once

#include "cache.h"

class EpochPredictorCache final : public Cache
{
public:
   EpochPredictorCache(UInt32 num_sets, UInt32 associativity, const String& replacement_policy);

   bool exists(IntPtr address);
   bool insert(IntPtr address);
   bool remove(IntPtr address);

   void print(const String& msg) const;
};

class EpochPredictor
{
public:
   EpochPredictor();
   ~EpochPredictor();

   LoggingPolicy predict(IntPtr address);
   bool predictNext(IntPtr address, float write_amplification);
   bool update(IntPtr address, LoggingPolicy new_policy);

private:
   EpochPredictorCache m_cache;
   UInt8 m_region_offset;
   std::pair<LoggingPolicy, LoggingPolicy> m_policy;

   static UInt32 getSize();
   static UInt32 getNumSets();
   static UInt32 getAssociativity();
   static String getReplacementPolicy();
   static UInt32 getRegionSize();
   static std::pair<LoggingPolicy, LoggingPolicy> getDefaultMode();
};
