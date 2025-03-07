#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_mru.h"
#include "cache_set_donuts.h"

namespace donuts
{

class CacheSetMRU final : public CacheSetDonuts, public ::CacheSetMRU
{
public:
   CacheSetMRU(CacheBase::cache_t cache_type,
               UInt32 index,
               UInt32 associativity,
               UInt32 threshold,
               UInt32 block_size);

   ~CacheSetMRU() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}
