#include "epoch_cntlr.h"
#include "checkpoint_event.h"

EpochCntlr::EpochCntlr(EpochManager& epoch_manager) :
    m_epoch_manager(epoch_manager)
{
}

EpochCntlr::~EpochCntlr() = default;

void
EpochCntlr::commit()
{
   CheckpointEvent event(CheckpointReason::PERIODIC_TIME);
   event.print();
}

void
EpochCntlr::newEpoch()
{
}