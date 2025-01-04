#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_random.h"
#include "cache_set_donuts.h"

namespace donuts
{

class CacheSetRandom final : public CacheSetDonuts, public ::CacheSetRandom
{
public:
   CacheSetRandom(CacheBase::cache_t cache_type,
                  UInt32 index,
                  UInt32 associativity,
                  UInt32 threshold,
                  UInt32 block_size);

   ~CacheSetRandom() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}
