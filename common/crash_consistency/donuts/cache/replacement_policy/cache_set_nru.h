#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_nru.h"
#include "cache_set.h"

namespace donuts
{

class CacheSetNRU final : public CacheSet, public ::CacheSetNRU
{
public:
   CacheSetNRU(CacheBase::cache_t cache_type,
               UInt32 index,
               UInt32 associativity,
               UInt32 threshold,
               UInt32 block_size);

   ~CacheSetNRU() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}

using CacheSetNRUDonuts = donuts::CacheSetNRU;
