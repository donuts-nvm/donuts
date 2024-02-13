#include "epoch_manager.h"

EpochManager::EpochManager()  : m_log_file(nullptr)
    , m_max_interval_time(SubsecondTime::NS(getMaxIntervalTime()))
    , m_max_interval_instr(getMaxIntervalInstructions())
    , m_cntlrs(getNumVersionedDomains())
    , m_cores_by_vd(getSharedCoresByVD())
{
   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_START, _start, (UInt64) this);
   Sim()->getHooksManager()->registerHook(HookType::HOOK_APPLICATION_EXIT, _exit, (UInt64) this);

   // --- Only for test! ---
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT, _commit, (UInt64) this);
   // Sim()->getHooksManager()->registerHook(HookType::HOOK_EPOCH_TIMEOUT_INS, _commit, (UInt64) this);

   registerStatsMetric("epoch", 0, "system_eid", &m_current.eid);
   registerStatsMetric("epoch", 0, "commited_eid", &m_commited.eid);
   registerStatsMetric("epoch", 0, "persisted_eid", &m_persisted.eid);

   createEpochCntlrs();
}

void EpochManager::createEpochCntlrs()
{
   std::vector<core_id_t> all_cores;
   auto total_cores = Sim()->getConfig()->getApplicationCores();
   all_cores.reserve((core_id_t) total_cores);
   for (core_id_t core_id = 0; core_id < (core_id_t) total_cores; core_id++)
      all_cores.push_back(core_id);

   if (m_cntlrs.size() == 1)
      m_cntlrs[0] = new EpochCntlr(this, 0, all_cores);

   else
   {
      for (std::vector<EpochCntlr*>::size_type vd_id = 0; vd_id < m_cntlrs.size(); vd_id++)
      {
         UInt32 first = vd_id * m_cores_by_vd;
         UInt32 last = (first + m_cores_by_vd) <= total_cores ? (first + m_cores_by_vd) : total_cores;
         std::vector<core_id_t> cores(all_cores.begin() + first, all_cores.begin() + last);
         m_cntlrs[vd_id] = new EpochCntlr(this, vd_id, cores);
      }
   }
}

EpochManager::~EpochManager()
{
   for (auto& m_cntlr : m_cntlrs)
      delete m_cntlr;
}

void EpochManager::start()
{
   const String filename = "sim.ckpts.csv";
   const String path = Sim()->getConfig()->getOutputDirectory() + "/" + filename.c_str();

   // if ((m_log_file = fopen(path.c_str(), "w")) == nullptr)
   // {
   //    fprintf(stderr, "Error on creating %s\n", filename.c_str());
   //    assert(m_log_file != nullptr);
   // }

   for (EpochCntlr* epoch_cntlr : m_cntlrs)
      epoch_cntlr->newEpoch();

   if (m_max_interval_time.getNS() > 0)
      Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC, _interrupt, (UInt64) this);
   if (m_max_interval_instr > 0)
      Sim()->getHooksManager()->registerHook(HookType::HOOK_PERIODIC_INS, _interrupt, (UInt64) this);

//   printf("EITA 0 1!\n");
   m_current.eid   = 1;
//   m_current.pc    = Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache();
//   m_current.instr = getTotalInstructionCount();
//   m_commited.time = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
   printf("EITA 0 2!\n");

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_START, m_current.eid);
}

void EpochManager::exit() const
{
   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_END, m_current.eid);

   // fclose(m_log_file);
}

void EpochManager::interrupt()
{
//   printf("t_now: %lu...\n", Sim()->getClockSkewMinimizationServer()->getGlobalTime().getNS());
   if (m_max_interval_time.getNS() > 0)
   {
      auto now = Sim()->getClockSkewMinimizationServer()->getGlobalTime();
      auto gap = gapBetweenCheckpoints<SubsecondTime>(now, getCommitedTime());
      if (gap >= m_max_interval_time)
         Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT, m_current.eid);
   }
   if (m_max_interval_instr > 0)
   {
      auto gap = gapBetweenCheckpoints<UInt64>(getTotalInstructionCount(), getCommitedInstruction());
      if (gap >= m_max_interval_instr)
         Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_TIMEOUT_INS, m_current.eid);
   }
}

void EpochManager::commit()
{
   m_commited.eid   = m_current.eid;
   m_commited.pc    = m_current.pc;
   m_commited.instr = getTotalInstructionCount();
   m_commited.time  = Sim()->getClockSkewMinimizationServer()->getGlobalTime();

   printf("Commited Epoch (%lu)\n"
          "- PC: %lX\n"
          "- Inst: %lu\n"
          "- Time: %lu\n",
          m_commited.eid,
          m_commited.pc,
          m_commited.instr,
          m_commited.time.getNS());

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_END, m_current.eid);

   m_current.instr = m_commited.instr;
   m_current.time  = m_commited.time;
   m_current.pc    = Sim()->getCoreManager()->getCurrentCore()->getLastPCToDCache();
   m_current.eid   = m_current.eid + 1;

   // fprintf(m_log_file, "%lu\n", m_commited.time.getNS());

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_START, m_current.eid);
}

void EpochManager::registerPersistedEID(UInt64 eid)
{
   m_persisted.eid = eid;

   Sim()->getHooksManager()->callHooks(HookType::HOOK_EPOCH_PERSISTED, eid);
}

UInt64 EpochManager::getTotalInstructionCount()
{
   UInt64 count = 0;
   for(core_id_t core_id = 0; core_id < (core_id_t) Sim()->getConfig()->getApplicationCores(); core_id++)
      count += Sim()->getCoreManager()->getCoreFromID(core_id)->getPerformanceModel()->getInstructionCount();
   return count;
}

template <typename T>
T EpochManager::gapBetweenCheckpoints(T current, T last) {
   return current >= last ? current - last : last - current;
}

UInt64 EpochManager::getMaxIntervalTime()
{
   const String key = "epoch/max_interval_time";

   SInt64 max_interval_time = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   assert(max_interval_time >= 0);

   if (max_interval_time != 0)
      assert(max_interval_time >= 1000); // (1000ns == 1μs)

   return (UInt64) max_interval_time;
}

UInt64 EpochManager::getMaxIntervalInstructions()
{
   const String key = "epoch/max_interval_instructions";

   SInt64 max_interval_instructions = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 0;
   assert(max_interval_instructions >= 0);

   if (max_interval_instructions != 0)
   {
      SInt64 ins_per_core = Sim()->getCfg()->getInt("core/hook_periodic_ins/ins_per_core");
      SInt64 ins_global = Sim()->getCfg()->getInt("core/hook_periodic_ins/ins_global");
      assert(max_interval_instructions >= ins_global);
      assert((ins_global >= ins_per_core) && (max_interval_instructions % ins_global == 0));
   }

   return (UInt64) max_interval_instructions;
}

UInt32 EpochManager::getNumVersionedDomains()
{
   const String key = "epoch/versioned_domains";
   SInt64 num_vds = Sim()->getCfg()->hasKey(key) ? Sim()->getCfg()->getInt(key) : 1;

   if (num_vds < 1)
   {
      auto total_cores = Sim()->getCfg()->getInt("general/total_cores");
      auto cores_by_vd = getSharedCoresByVD();
      num_vds = std::ceil(static_cast<double>(total_cores) / cores_by_vd);
   }

   return num_vds;
}

/**
 * Deduces the number of cores in each Versioned Domain checking the number
 * of shared cores in the penultimate level cache
 *
 * @return UInt32
 */
UInt32 EpochManager::getSharedCoresByVD()
{
   auto cache_levels = Sim()->getCfg()->getInt("perf_model/cache/levels");
   assert(cache_levels >= 2);

   auto vd_level = cache_levels - 1;
   String penult_cache = vd_level == 1 ? "l1_dcache" : "l" + String(std::to_string(vd_level).c_str()) + "_cache";
   return Sim()->getCfg()->getInt("perf_model/" + penult_cache + "/shared_cores");
}

EpochCntlr* EpochManager::getEpochCntlr(const core_id_t core_id)
{
   if (m_cntlrs.size() == 1)
      return m_cntlrs[0];

   return m_cntlrs[core_id / m_cores_by_vd];
}

UInt64 EpochManager::getGlobalSystemEID()
{
   return Sim()->getEpochManager()->getSystemEID();
}

EpochManager *EpochManager::getInstance()
{
   return Sim()->getEpochManager();
}