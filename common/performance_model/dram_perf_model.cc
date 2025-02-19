#include "simulator.h"
#include "dram_perf_model.h"
#include "dram_perf_model_constant.h"
#include "dram_perf_model_readwrite.h"
#include "dram_perf_model_normal.h"
#include "config.hpp"

DramPerfModel* DramPerfModel::createDramPerfModel(core_id_t core_id, UInt32 cache_block_size)
{
   String type = Sim()->getCfg()->getString("perf_model/dram/type");

   if (type == "constant")
   {
      return new DramPerfModelConstant(core_id, cache_block_size);
   }
   else if (type == "readwrite")
   {
      return new DramPerfModelReadWrite(core_id, cache_block_size);
   }
   else if (type == "normal")
   {
      return new DramPerfModelNormal(core_id, cache_block_size);
   }
   else
   {
      LOG_PRINT_ERROR("Invalid DRAM model type %s", type.c_str());
   }
}

float DramPerfModel::loadDramConfig(const String& param)
{
   if (const auto key = "perf_model/dram/" + param; Sim()->getCfg()->hasKey(key))
      printf("BUCETA 1 [%s]: %.1f\n", key.c_str(), Sim()->getCfg()->getFloat(key));

   if (const auto key = "perf_model/dram/" + param; Sim()->getCfg()->hasKey(key))
      return static_cast<float>(Sim()->getCfg()->getFloat(key));

   if (String key = "perf_model/dram/protocol"; Sim()->getCfg()->hasKey(key))
   {
      const auto protocol = Sim()->getCfg()->getString(key);
      printf("BUCETA 2: %s\n", protocol.c_str());
      if (key = "perf_model/" + protocol + "/" + param; Sim()->getCfg()->hasKey(key))
         return static_cast<float>(Sim()->getCfg()->getFloat(key));
   }

   LOG_PRINT_ERROR("perf_model/dram/%s not found", param.c_str());
}

float DramPerfModel::loadBandwidth()
{
   const float bandwidth = loadDramConfig("per_controller_bandwidth");
   LOG_ASSERT_ERROR(bandwidth >= 0, "Invalid 'perf_model/dram/per_controller_bandwidth' value: %.2f GB/s", bandwidth);

   return 8 * bandwidth; // Convert bytes to bits
}

SubsecondTime
DramPerfModel::loadLatency()
{
   const float latency = loadDramConfig("latency");
   LOG_ASSERT_ERROR(latency >= 0, "Invalid 'perf_model/dram/latency' value: %.1f ns", latency);

   return SubsecondTime::FS() * static_cast<uint64_t>(TimeConverter<float>::NStoFS(latency)); // Operate in fs for higher precision before converting to uint64_t/SubsecondTime
}
