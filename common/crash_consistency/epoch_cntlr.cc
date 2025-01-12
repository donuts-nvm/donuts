#include "epoch_cntlr.h"

EpochCntlr::EpochCntlr(EpochManager& epoch_manager) :
    m_epoch_manager(epoch_manager)
{
}

EpochCntlr::~EpochCntlr() = default;
