#pragma once

class EpochManager;

class EpochCntlr
{
public:
   explicit EpochCntlr(EpochManager& epoch_manager);
   ~EpochCntlr();

   void newEpoch();

   void commit();

private:
   EpochManager& m_epoch_manager;
};
