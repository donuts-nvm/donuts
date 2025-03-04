#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_lru.h"
#include "cache_set.h"

namespace donuts
{

class CacheSetLRU final : public CacheSet, public ::CacheSetLRU
{
public:
   CacheSetLRU(CacheBase::cache_t cache_type,
               UInt32 index,
               UInt32 associativity,
               UInt32 threshold,
               UInt32 block_size,
               CacheSetInfoLRU* set_info,
               UInt8 num_attempts);

   ~CacheSetLRU() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}

using CacheSetLRUDonuts = donuts::CacheSetLRU;
