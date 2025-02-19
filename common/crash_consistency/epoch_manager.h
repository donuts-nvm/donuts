#pragma once

#include "epoch_cntlr.h"
#include "subsecond_time.h"

#include <memory>
#include <optional>
#include <vector>

class EpochManager
{
public:
   ~EpochManager();

   EpochManager(const EpochManager&)            = delete;
   EpochManager(EpochManager&&)                 = delete;
   EpochManager& operator=(const EpochManager&) = delete;
   EpochManager& operator=(EpochManager&&)      = delete;

   [[nodiscard]] EpochCntlr& getEpochCntlr(const core_id_t core_id) const { return *m_cntlrs_map[core_id]; }

   static UInt64 getCurrentEID(core_id_t = -1);
   // static UInt64 getPersistedEID(core_id_t = -1);

   static std::unique_ptr<EpochManager> create();

private:
   struct VDConfig
   {
      static VDConfig create(UInt32 total_cores, const SubsecondTime& max_interval_time, UInt64 max_interval_instr);
      static VDConfig create(core_id_t first_core, core_id_t last_core, const SubsecondTime& max_interval_time, UInt64 max_interval_instr);

      std::vector<core_id_t> cores{};
      SubsecondTime max_interval_time{ SubsecondTime::Zero() };
      UInt64 max_interval_instr{ 0 };
   };

   explicit EpochManager(bool multi_domains = false);

   std::vector<std::unique_ptr<EpochCntlr>> m_cntlrs;
   std::vector<EpochCntlr*> m_cntlrs_map;

   void start() const;
   void end() const;

   static UInt32 loadNumVersionedDomains();
   static std::vector<UInt32> loadSharedCoresByVD();
   static std::optional<UInt16> loadSimilarToLevel();
   static std::vector<SubsecondTime> loadMaxIntervalTime();
   static std::vector<UInt64> loadMaxIntervalInstructions();

   static std::vector<UInt32> getVDRanges(bool multi_domains);
   static std::vector<VDConfig> getVDConfigs(bool multi_domains);
};
