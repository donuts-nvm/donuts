#include "epoch_manager.h"
#include "config.hpp"
#include "core_manager.h"
#include "hooks_manager.h"
#include "simulator.h"

#include <format>
#include <numeric>

EpochManager::VDConfig
EpochManager::VDConfig::create(const UInt32 total_cores, const SubsecondTime& max_interval_time, const UInt64 max_interval_instr)
{
   return create(0, static_cast<core_id_t>(total_cores) - 1, max_interval_time, max_interval_instr);
}

EpochManager::VDConfig
EpochManager::VDConfig::create(const core_id_t first_core, const core_id_t last_core,
                               const SubsecondTime& max_interval_time, const UInt64 max_interval_instr)
{
   LOG_ASSERT_ERROR(first_core <= last_core, "The first_core must be equals or less than last_core [first_core = %u, last_core = %u]",
                    first_core, last_core);
   std::vector<core_id_t> cores(last_core - first_core + 1);
   std::iota(cores.begin(), cores.end(), first_core);

   VDConfig config;
   config.cores              = cores;
   config.max_interval_time  = max_interval_time;
   config.max_interval_instr = max_interval_instr;

   return config;
}

EpochManager::EpochManager(const bool multi_domains)
{
   for (core_id_t core_id = 0, vd_index = 0; const auto& [cores, max_time, max_instr]: getVDConfigs(multi_domains))
   {
      m_cntlrs.emplace_back(std::make_unique<EpochCntlr>(*this, vd_index++, cores.front(), cores.back(), max_time, max_instr, cores));
      for (core_id_t i = core_id; i <= cores.back(); ++i)
      {
         m_cntlrs_map.push_back(m_cntlrs.back().get());
      }
   }

   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_START, [this](UInt64) { start(); });
   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_EXIT, [this](UInt64) { end(); });
}

EpochManager::~EpochManager()
{
   m_cntlrs.clear();
}

void
EpochManager::start() const
{
   for (const auto& epoch_cntlr: m_cntlrs)
   {
      epoch_cntlr->newEpoch();
   }
}

void
EpochManager::end() const
{
   // TODO: Emitir relatório de cada epoch controller
}

UInt64
EpochManager::getCurrentEID(core_id_t core_id)
{
   core_id = core_id < 0 ? Sim()->getCoreManager()->getCurrentCoreID() : core_id;
   LOG_ASSERT_ERROR(core_id >= 0 && core_id < static_cast<core_id_t>(Sim()->getConfig()->getTotalCores()),
                    "cores[%d] out of range", core_id);

   return Sim()->getEpochManager().getEpochCntlr(core_id).getCurrentEID();
}

// UInt64
// EpochManager::getPersistedEID(core_id_t core_id)
// {
//    core_id = core_id < 0 ? Sim()->getCoreManager()->getCurrentCoreID() : core_id;
//    LOG_ASSERT_ERROR(core_id >= 0 && core_id < static_cast<core_id_t>(Sim()->getConfig()->getTotalCores()),
//                     "cores[%d] out of range", core_id);
//
//    return Sim()->getEpochManager().getEpochCntlr(core_id).getPersistedEID();
// }

std::vector<UInt32>
EpochManager::getVDRanges(const bool multi_domains)
{
   auto num_vds           = loadNumVersionedDomains();
   auto shared_cores      = loadSharedCoresByVD();
   const auto total_cores = Sim()->getConfig()->getTotalCores();

   if (!multi_domains) return { total_cores };

   if (num_vds == 0)
   {
      if (shared_cores.size() > 1)
      {
         num_vds = shared_cores.size();
      }
      else if (shared_cores[0] > 0)
      {
         LOG_ASSERT_ERROR(total_cores % shared_cores[0] == 0,
                          "Invalid configuration. Use 'epoch/shared_cores' to specify the distribution of cores per versioned domains");
         num_vds = total_cores / shared_cores[0];
      }
      else
      {
         if (const auto level = loadSimilarToLevel(); level.has_value())
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
         {
            num_vds = total_cores;
         }
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
      {
         shared_cores.resize(num_vds, shared_cores[0]);
      }
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

   const auto& ranges             = getVDRanges(multi_domains);
   const auto& max_interval_times = loadMaxIntervalTime();
   const auto& max_interval_instr = loadMaxIntervalInstructions();

   LOG_ASSERT_WARNING(max_interval_times.size() == 1 || max_interval_times.size() == ranges.size(),
                      "The number of max interval time [%u] does not match the number of versioned domains [%u]",
                      max_interval_times.size(), ranges.size());

   LOG_ASSERT_WARNING(max_interval_instr.size() == 1 || max_interval_instr.size() == ranges.size(),
                      "The number of max interval instructions [%u] does not match the number of versioned domains [%u]",
                      max_interval_instr.size(), ranges.size());

   for (core_id_t core_index = 0, i = 0; i < static_cast<core_id_t>(ranges.size()); i++)
   {
      const auto num_cores = static_cast<core_id_t>(ranges[i]);
      const auto max_time  = max_interval_times.size() == 1 ? max_interval_times[0] : max_interval_times[i];
      const auto max_instr = max_interval_instr.size() == 1 ? max_interval_instr[0] : max_interval_instr[i];
      configs.emplace_back(VDConfig::create(core_index, core_index + num_cores - 1, max_time, max_instr));
      core_index += num_cores;
   }

   return configs;
}

UInt32
EpochManager::loadNumVersionedDomains()
{
   const auto* key = "epoch/versioned_domains";

   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   LOG_ASSERT_ERROR(value >= 0 && value <= UINT32_MAX, "Invalid value for '%s'. Use from 0 to UINT32_MAX", key);

   return value;
}

std::vector<UInt32>
EpochManager::loadSharedCoresByVD()
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
EpochManager::loadSimilarToLevel()
{
   const auto* key = "epoch/similar_to_level";

   if (!Sim()->getCfg()->hasKey(key)) return std::nullopt;

   const auto level     = Sim()->getCfg()->getInt(key);
   const auto max_level = Sim()->getCfg()->getInt("perf_model/cache/levels");
   LOG_ASSERT_ERROR(level >= 0 && level <= max_level, "Invalid value for '%s'. Use 0 from %u]", key, max_level);

   return static_cast<UInt16>(level);
}

std::vector<SubsecondTime>
EpochManager::loadMaxIntervalTime()
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
EpochManager::loadMaxIntervalInstructions()
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

std::unique_ptr<EpochManager>
EpochManager::create()
{
   switch (Sim()->getProjectType())
   {
      case ProjectType::BASELINE:
         return nullptr;
      case ProjectType::DONUTS:
         return std::unique_ptr<EpochManager>(new EpochManager);
      default:
         LOG_PRINT_ERROR("Unsupported project type: '%s'", Sim()->getProjectName());
   }
}
