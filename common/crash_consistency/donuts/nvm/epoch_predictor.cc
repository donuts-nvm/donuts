#include "epoch_predictor.h"
#include "config.hpp"
#include "simulator.h"

#include <cache_set_srrip.h>
#include <cmath>
#include <format>

EpochPredictorCache::EpochPredictorCache(const core_id_t core_id,
                                         const UInt32 num_sets,
                                         const UInt32 associativity,
                                         const String& replacement_policy) :
    Cache(NAME, CONFIG_NAME, core_id, num_sets, associativity, 1, replacement_policy, SHARED_CACHE) {}

bool
EpochPredictorCache::exists(const IntPtr address)
{
   return peekSingleLine(address) != nullptr;
}

bool
EpochPredictorCache::insert(const IntPtr address)
{
   if (!exists(address))
   {
      Byte data_buff, evict_byte;
      bool eviction;
      IntPtr evict_addr;
      CacheBlockInfo evict_block_info;

      insertSingleLine(address, &data_buff, &eviction, &evict_addr, &evict_block_info, &evict_byte, SubsecondTime::Zero());
      return true;
   }
   return false;
}

bool
EpochPredictorCache::remove(const IntPtr address)
{
   if (exists(address))
   {
      invalidateSingleLine(address);
      return true;
   }
   return false;
}

void
EpochPredictorCache::print(const String& msg) const
{
   if (!msg.empty())
   {
      printf("--------------------------------------------------------------------------\n");
      printf(" %s\n", msg.c_str());
      printf("--------------------------------------------------------------------------\n");
   }

   const UInt32 associativity = getAssociativity();

   printf("%-5s", "Index");
   for (UInt32 j = 0; j < associativity; j++) printf("   %13s ", std::format("Way {}", j).c_str());
   printf("\n--------------------------------------------------------------------------\n");

   for (UInt32 i = 0; i < getNumSets(); i++)
   {
      printf("%5u: ", i);
      for (UInt32 j = 0; j < associativity; j++)
      {
         if (const auto* block = peekBlock(i, j); block->isValid())
            printf("[ %012lx ] ", tagToAddress(block->getTag()));
         else
            printf("[ %12s ] ", "");
      }
      printf("\n");
   }
}

EpochPredictor::EpochPredictor(const core_id_t core_id) :
    m_cache(core_id, getNumSets(), getAssociativity(), getReplacementPolicy()),
    m_region_offset(static_cast<UInt8>(std::log2(getRegionSize()))),
    m_policy(getDefaultMode())
{
   m_cache.print("STATE 0");

   update(64, LoggingPolicy::LOGGING_ON_READ);
   update(65, LoggingPolicy::LOGGING_ON_READ);
   update(100, LoggingPolicy::LOGGING_ON_READ);
   m_cache.print("STATE 1");

   update(98, LoggingPolicy::LOGGING_ON_WRITE);
   m_cache.print("CACHE 2");

   update(110, LoggingPolicy::LOGGING_ON_READ);
   update(128, LoggingPolicy::LOGGING_ON_READ);
   m_cache.print("CACHE 3");

   printf("Qual para [%lu] ? [%i]\n", 64L, static_cast<int>(predict(64)));
   printf("Qual para [%lu] ? [%i]\n", 88L, static_cast<int>(predict(88)));
   printf("Qual para [%lu] ? [%i]\n", 130L, static_cast<int>(predict(130)));
   printf("Qual para [%lu] ? [%i]\n", 1000L, static_cast<int>(predict(1000)));
}

EpochPredictor::~EpochPredictor() = default;

LoggingPolicy
EpochPredictor::predict(const IntPtr address)
{
   return m_cache.exists(address >> m_region_offset) ? m_policy.second : m_policy.first;
}

bool
EpochPredictor::update(const IntPtr address, const LoggingPolicy new_policy)
{
   if (new_policy == m_policy.second)
      return m_cache.insert(address >> m_region_offset);

   return m_cache.remove(address >> m_region_offset);
}

UInt32
EpochPredictor::getSize()
{
   const auto* key  = "perf_model/epoch_predictor/size";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : UINT32_MAX;
   LOG_ASSERT_ERROR(value >= 0, "Invalid value for '%s': %l", key, value);

   return static_cast<UInt32>(value == 0 ? UINT32_MAX : value);
}

UInt32
EpochPredictor::getNumSets()
{
   const auto size          = getSize();
   const auto associativity = getAssociativity();
   LOG_ASSERT_ERROR(size >= associativity && size % associativity == 0,
                    "The epoch predictor size (%u) is not compatible to it associativity (%u)", size, associativity);
   return size / associativity;
}

UInt32
EpochPredictor::getAssociativity()
{
   const auto* key  = "perf_model/epoch_predictor/associativity";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 4;
   LOG_ASSERT_ERROR(value > 0, "Invalid value for '%s': %ld", key, value);

   return static_cast<UInt32>(value);
}

String
EpochPredictor::getReplacementPolicy()
{
   const auto* key  = "perf_model/epoch_predictor/replacement_policy";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getString(key) : "low";
   CacheSet::parsePolicyType(value);

   return value;
}

UInt32
EpochPredictor::getRegionSize()
{
   const auto* key  = "perf_model/epoch_predictor/region_size";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 64;
   LOG_ASSERT_ERROR(value > 0 && std::has_single_bit(static_cast<UInt32>(value)), "Invalid value for '%s': %ld", key, value);

   return static_cast<UInt32>(value);
}

std::pair<LoggingPolicy, LoggingPolicy>
EpochPredictor::getDefaultMode()
{
   const auto* key  = "perf_model/epoch_predictor/default_mode";
   const auto value = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getString(key) : "low";

   const auto default_policy = LogPolicy::fromType(value);
   LOG_ASSERT_ERROR(default_policy == LoggingPolicy::LOGGING_ON_READ || default_policy == LoggingPolicy::LOGGING_ON_WRITE,
                    "Invalid value for '%s': '%s'", key, value.c_str());

   const auto alternative_policy = default_policy == LoggingPolicy::LOGGING_ON_WRITE ?
                                         LoggingPolicy::LOGGING_ON_READ :
                                         LoggingPolicy::LOGGING_ON_WRITE;

   return std::make_pair(default_policy, alternative_policy);
}
