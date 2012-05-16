#include "config.h"

#include "network_model.h"
#include "packet_type.h"
#include "simulator.h"
#include "utils.h"
#include "config.hpp"

#include <sstream>

#define DEBUG

String Config::m_knob_output_directory;
UInt32 Config::m_knob_total_cores;
bool Config::m_knob_enable_icache_modeling;
bool Config::m_knob_use_magic;
bool Config::m_knob_enable_progress_trace;
bool Config::m_knob_enable_sync;
bool Config::m_knob_enable_sync_report;
bool Config::m_knob_osemu_pthread_replace;
UInt32 Config::m_knob_osemu_nprocs;
bool Config::m_knob_osemu_clock_replace;
time_t Config::m_knob_osemu_time_start;
bool Config::m_knob_bbvs;
bool Config::m_knob_enable_perbasicblock;
ClockSkewMinimizationObject::Scheme Config::m_knob_clock_skew_minimization_scheme;

Config *Config::m_singleton;

Config *Config::getSingleton()
{
   assert(m_singleton != NULL);
   return m_singleton;
}

// Don't call LOG_* inside Config::Config, as the logging infrastructure isn't set up yet at this point
#undef LOG_PRINT_ERROR
#undef LOG_ASSERT_ERROR

Config::Config(SimulationMode mode)
{
   // NOTE: We can NOT use logging in the config constructor! The log
   // has not been instantiated at this point!

   m_knob_output_directory = Sim()->getCfg()->getString("general/output_dir");
   m_knob_total_cores = Sim()->getCfg()->getInt("general/total_cores");

   m_knob_enable_icache_modeling = Sim()->getCfg()->getBool("general/enable_icache_modeling");

   m_knob_use_magic = Sim()->getCfg()->getBool("general/magic");
   m_knob_enable_progress_trace = Sim()->getCfg()->getBool("progress_trace/enabled");
   m_knob_enable_sync = Sim()->getCfg()->getString("clock_skew_minimization/scheme") != "none";
   m_knob_enable_sync_report = Sim()->getCfg()->getBool("clock_skew_minimization/report");

   m_simulation_mode = mode;
   m_knob_bbvs = false; // No config setting here, but enabled by code (BBVSamplingProvider, [py|lua]_bbv) that needs it

   // OS Emulation flags
   m_knob_osemu_pthread_replace = Sim()->getCfg()->getBool("osemu/pthread_replace");
   m_knob_osemu_nprocs = Sim()->getCfg()->getInt("osemu/nprocs");
   m_knob_osemu_clock_replace = Sim()->getCfg()->getBool("osemu/clock_replace");
   m_knob_osemu_time_start = Sim()->getCfg()->getInt("osemu/time_start");

   m_knob_clock_skew_minimization_scheme = ClockSkewMinimizationObject::parseScheme(Sim()->getCfg()->getString("clock_skew_minimization/scheme"));

   m_total_cores = m_knob_total_cores;

   m_singleton = this;

   assert(m_total_cores > 0);

   //No, be sure to configure a valid NumApplicationCores count
   //// Adjust the number of cores corresponding to the network model we use
   //m_total_cores = getNearestAcceptableCoreCount(m_total_cores);

   m_core_id_length = computeCoreIDLength(m_total_cores);
}

Config::~Config()
{
}

UInt32 Config::getTotalCores()
{
   return m_total_cores;
}

UInt32 Config::getApplicationCores()
{
   return getTotalCores();
}

UInt32 Config::computeCoreIDLength(UInt32 core_count)
{
   UInt32 num_bits = ceilLog2(core_count);
   if ((num_bits % 8) == 0)
      return (num_bits / 8);
   else
      return (num_bits / 8) + 1;
}

// Parse XML config file and use it to fill in config state.  Only modifies
// fields specified in the config file.  Therefore, this method can be used
// to override only the specific options given in the file.
void Config::loadFromFile(char* filename)
{
   return;
}

// Fill in config state from command-line arguments.  Only modifies fields
// specified on the command line.  Therefore, this method can be used to
// override only the specific options given.
void Config::loadFromCmdLine()
{
   return;
}

String Config::getOutputDirectory() const
{
   return m_knob_output_directory;
}

String Config::formatOutputFileName(String filename) const
{
   return m_knob_output_directory + "/" + filename;
}

void Config::updateCommToCoreMap(UInt32 comm_id, core_id_t core_id)
{
   m_comm_to_core_map[comm_id] = core_id;
}

UInt32 Config::getCoreFromCommId(UInt32 comm_id)
{
   CommToCoreMap::iterator it = m_comm_to_core_map.find(comm_id);
   return it == m_comm_to_core_map.end() ? INVALID_CORE_ID : it->second;
}

void Config::getNetworkModels(UInt32 *models) const
{
   try
   {
      config::Config *cfg = Sim()->getCfg();
      models[STATIC_NETWORK_USER_1] = NetworkModel::parseNetworkType(cfg->getString("network/user_model_1"));
      models[STATIC_NETWORK_USER_2] = NetworkModel::parseNetworkType(cfg->getString("network/user_model_2"));
      models[STATIC_NETWORK_MEMORY_1] = NetworkModel::parseNetworkType(cfg->getString("network/memory_model_1"));
      models[STATIC_NETWORK_MEMORY_2] = NetworkModel::parseNetworkType(cfg->getString("network/memory_model_2"));
      models[STATIC_NETWORK_SYSTEM] = NetworkModel::parseNetworkType(cfg->getString("network/system_model"));
   }
   catch (...)
   {
      config::Error("Exception while reading network model types.");
   }
}

UInt32 Config::getNearestAcceptableCoreCount(UInt32 core_count)
{
   UInt32 nearest_acceptable_core_count = 0;

   UInt32 l_models[NUM_STATIC_NETWORKS];
   try
   {
      config::Config *cfg = Sim()->getCfg();
      l_models[STATIC_NETWORK_USER_1] = NetworkModel::parseNetworkType(cfg->getString("network/user_model_1"));
      l_models[STATIC_NETWORK_USER_2] = NetworkModel::parseNetworkType(cfg->getString("network/user_model_2"));
      l_models[STATIC_NETWORK_MEMORY_1] = NetworkModel::parseNetworkType(cfg->getString("network/memory_model_1"));
      l_models[STATIC_NETWORK_MEMORY_2] = NetworkModel::parseNetworkType(cfg->getString("network/memory_model_2"));
      l_models[STATIC_NETWORK_SYSTEM] = NetworkModel::parseNetworkType(cfg->getString("network/system_model"));
   }
   catch (...)
   {
      config::Error("Exception while reading network model types.");
   }

   for (UInt32 i = 0; i < NUM_STATIC_NETWORKS; i++)
   {
      std::pair<bool,SInt32> core_count_constraints = NetworkModel::computeCoreCountConstraints(l_models[i], (SInt32) core_count);
      if (core_count_constraints.first)
      {
         // Network Model has core count constraints
         if ((nearest_acceptable_core_count != 0) &&
             (core_count_constraints.second != (SInt32) nearest_acceptable_core_count))
         {
            config::Error("Problem using the network models specified in the configuration file.");
         }
         else
         {
            nearest_acceptable_core_count = core_count_constraints.second;
         }
      }
   }

   if (nearest_acceptable_core_count == 0)
      nearest_acceptable_core_count = core_count;

   return nearest_acceptable_core_count;
}
