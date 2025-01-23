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

   [[nodiscard]] EpochCntlr& getEpochCntlr(const core_id_t core_id) const { return *m_cntlrs[core_id]; }

   static std::unique_ptr<EpochManager> create();

private:
   explicit EpochManager(bool multi_domains = false);

   struct VDConfig
   {
      std::vector<core_id_t> cores{};
      SubsecondTime max_interval_time{ SubsecondTime::Zero() };
      UInt64 max_interval_instr{ 0 };
   };

   std::vector<EpochCntlr*> m_cntlrs;

   void start();
   void end();

   static UInt32 getNumVersionedDomains();
   static std::vector<UInt32> getSharedCoresByVD();
   static std::optional<UInt16> getSimilarToLevel();
   static std::vector<SubsecondTime> getMaxIntervalTime();
   static std::vector<UInt64> getMaxIntervalInstructions();

   static std::vector<UInt32> getVDRanges(bool multi_domains);
   static std::vector<VDConfig> getVDConfigs(bool multi_domains);
};
