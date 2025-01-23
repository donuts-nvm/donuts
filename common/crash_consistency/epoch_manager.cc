#include "epoch_manager.h"
#include "config.hpp"
#include "hooks_manager.h"
#include "simulator.h"
#include "watchdog.h"

#include <algorithm>
#include <format>
#include <numeric>

EpochManager::EpochManager(const bool multi_domains)
{
   const auto _start = [](const UInt64 this_arg, UInt64) -> SInt64
   {
      reinterpret_cast<EpochManager*>(this_arg)->start();
      return 0;
   };
   const auto _exit = [](const UInt64 this_arg, UInt64) -> SInt64
   {
      reinterpret_cast<EpochManager*>(this_arg)->end();
      return 0;
   };
   const auto this_ptr = reinterpret_cast<UInt64>(this);

   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_START, _start, this_ptr);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_EXIT, _exit, this_ptr);

   if (const auto& vds = getVDConfigs(multi_domains); vds.size() == 1)
   {
      m_cntlrs.push_back(new EpochCntlr(*this, 0, vds[0].max_interval_time, vds[0].max_interval_instr,
                                        Sim()->getConfig()->getTotalCores()));
   }
   else
   {
      core_id_t core_id = 0, vd_index = 0;
      for (const auto& [cores, max_time, max_instr]: vds)
      {
         auto* cntrl = new EpochCntlr(*this, vd_index++, max_time, max_instr, cores);
         do m_cntlrs.push_back(cntrl);
         while (++core_id <= cores.back());
      }

      printf("[ ");
      for (UInt32 i = 0; i < Sim()->getConfig()->getTotalCores(); i++) printf("%p ", m_cntlrs[i]);
      printf(" ]\n");
   }
}

EpochManager::~EpochManager()
{
   printf("DESTRUINDO EPOCH MANAGER\n");
   for (const EpochCntlr* cntlr = nullptr; const auto* ptr: m_cntlrs)
   {
      if (ptr != cntlr)
      {
         cntlr = ptr;
         delete ptr;
      }
   }
}

void
EpochManager::start()
{
   for (EpochCntlr* epoch_cntlr: m_cntlrs)
      epoch_cntlr->newEpoch();
}

void
EpochManager::end()
{
}

UInt32
EpochManager::getNumVersionedDomains()
{
   const auto* key = "epoch/versioned_domains";

   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   LOG_ASSERT_ERROR(value >= 0 && value <= UINT32_MAX, "Invalid value for '%s'. Use from 0 to UINT32_MAX", key);

   return value;
}

std::vector<UInt32>
EpochManager::getSharedCoresByVD()
{
   const auto* key = "epoch/shared_cores";
   std::vector<UInt32> shared_cores;

   if (!Sim()->getCfg()->hasKey(key)) return { 0 };

   for (UInt64 i = 0; i < Sim()->getCfg()->getArraySize(key); i++)
   {
      const auto value = Sim()->getCfg()->getIntArray(key, i);
      LOG_ASSERT_ERROR(value >= 0 && value <= INT64_MAX, "Invalid value for '%s'. Use from 0 to INT64_MAX", key);
      shared_cores.push_back(static_cast<UInt32>(value));
   }

   return shared_cores;
}

std::optional<UInt16>
EpochManager::getSimilarToLevel()
{
   const auto* key = "epoch/similar_to_level";

   if (!Sim()->getCfg()->hasKey(key)) return std::nullopt;

   const auto level     = Sim()->getCfg()->getInt(key);
   const auto max_level = Sim()->getCfg()->getInt("perf_model/cache/levels");
   LOG_ASSERT_ERROR(level >= 0 && level <= max_level, "Invalid value for '%s'. Use 0 from %u]", key, max_level);

   return static_cast<UInt16>(level);
}

std::vector<SubsecondTime>
EpochManager::getMaxIntervalTime()
{
   const auto* key = "epoch/max_interval_time";
   std::vector<SubsecondTime> max_interval;

   if (!Sim()->getCfg()->hasKey(key)) return { SubsecondTime::Zero() };

   for (UInt64 i = 0; Sim()->getCfg()->hasKey(key) && i < Sim()->getCfg()->getArraySize(key); i++)
   {
      const auto value = Sim()->getCfg()->getIntArray(key, i);
      LOG_ASSERT_ERROR(value == 0 || (value >= 1000 && value <= INT64_MAX),
                       "Invalid value for '%s'. Use 0 (zero) or a value greater than or equal to 1000 (1μs)", key);

      max_interval.push_back(SubsecondTime::NS(value));
   }

   return max_interval;
}

std::vector<UInt64>
EpochManager::getMaxIntervalInstructions()
{
   const auto* key = "epoch/max_interval_instructions";
   std::vector<UInt64> max_interval;

   if (!Sim()->getCfg()->hasKey(key)) return { 0 };

   for (UInt64 i = 0; i < Sim()->getCfg()->getArraySize(key); i++)
   {
      const auto value = Sim()->getCfg()->getIntArray(key, i);
      LOG_ASSERT_ERROR(value >= 0 && value <= INT64_MAX, "Invalid value for '%s'. Use from 0 to INT64_MAX", key);
      max_interval.push_back(static_cast<UInt64>(value));
   }

   return max_interval;
}

std::vector<UInt32>
EpochManager::getVDRanges(const bool multi_domains)
{
   auto num_vds           = getNumVersionedDomains();
   auto shared_cores      = getSharedCoresByVD();
   const auto total_cores = Sim()->getConfig()->getTotalCores();

   if (!multi_domains) return { total_cores };

   if (num_vds == 0)
   {
      if (shared_cores.size() > 1)
         num_vds = shared_cores.size();
      else if (shared_cores[0] > 0)
      {
         LOG_ASSERT_ERROR(total_cores % shared_cores[0] == 0,
                          "Invalid configuration. Use 'epoch/shared_cores' to specify the distribution of cores per versioned domains");
         num_vds = total_cores / shared_cores[0];
      }
      else
      {
         if (const auto level = getSimilarToLevel(); level.has_value())
         {
            UInt32 shared = 1;
            if (level.value() > 0)
            {
               const auto c_level = level.value() == 1 ? "l1_dcache" : std::format("l{}_cache", level.value());
               shared             = Sim()->getCfg()->getInt(std::format("perf_model/{}/shared_cores", c_level).c_str());
            }
            num_vds      = total_cores / shared;
            shared_cores = std::vector(num_vds, shared);
         }
         else
            num_vds = total_cores;
      }
   }

   if (shared_cores.size() == 1)
   {
      if (shared_cores[0] == 0)
      {
         const UInt32 shared = total_cores / num_vds;
         shared_cores        = std::vector(num_vds, shared);
      }
      else
         shared_cores.resize(num_vds, shared_cores[0]);
   }

   LOG_ASSERT_ERROR(shared_cores.size() == num_vds,
                    "The shared cores array size [%u] does not match the number of versioned domains [%u]",
                    shared_cores.size(), num_vds);
   LOG_ASSERT_ERROR(total_cores == std::accumulate(shared_cores.begin(), shared_cores.end(), static_cast<UInt32>(0)),
                    "The domain configuration does not match the total cores");

   return shared_cores;
}

std::vector<EpochManager::VDConfig>
EpochManager::getVDConfigs(const bool multi_domains)
{
   std::vector<VDConfig> configs;
   UInt32 core_index = 0;

   const auto& ranges             = getVDRanges(multi_domains);
   const auto& max_interval_times = getMaxIntervalTime();
   const auto& max_interval_instr = getMaxIntervalInstructions();

   LOG_ASSERT_WARNING(max_interval_times.size() == 1 || max_interval_times.size() == ranges.size(),
                      "The number of max interval time [%u] does not match the number of versioned domains [%u]",
                      max_interval_times.size(), ranges.size());

   LOG_ASSERT_WARNING(max_interval_instr.size() == 1 || max_interval_instr.size() == ranges.size(),
                      "The number of max interval instructions [%u] does not match the number of versioned domains [%u]",
                      max_interval_instr.size(), ranges.size());

   for (std::size_t i = 0; i < ranges.size(); i++)
   {
      VDConfig config;
      config.cores.resize(ranges[i]);
      std::ranges::iota(config.cores, core_index);
      core_index += ranges[i];
      config.max_interval_time  = max_interval_times.size() == 1 ? max_interval_times[0] : max_interval_times[i];
      config.max_interval_instr = max_interval_instr.size() == 1 ? max_interval_instr[0] : max_interval_instr[i];
      configs.push_back(config);
   }

   return configs;
}

std::unique_ptr<EpochManager>
EpochManager::create()
{
   switch (Sim()->getProjectType())
   {
      case ProjectType::BASELINE:
         return nullptr;
      case ProjectType::DONUTS:
         // return std::unique_ptr<EpochManager>(new EpochManager(false));
         return std::unique_ptr<EpochManager>(new EpochManager(true));
      default:
         LOG_PRINT_ERROR("Unsupported project type");
   }
}
