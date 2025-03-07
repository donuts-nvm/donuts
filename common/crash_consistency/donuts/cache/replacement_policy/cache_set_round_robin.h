#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_round_robin.h"
#include "cache_set_donuts.h"

namespace donuts
{

class CacheSetRoundRobin final : public CacheSetDonuts, public ::CacheSetRoundRobin
{
public:
   CacheSetRoundRobin(CacheBase::cache_t cache_type,
                      UInt32 index,
                      UInt32 associativity,
                      UInt32 threshold,
                      UInt32 block_size);

   ~CacheSetRoundRobin() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}
