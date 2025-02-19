#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_nmru.h"
#include "cache_set.h"

namespace donuts
{

class CacheSetNMRU final : public CacheSet, public ::CacheSetNMRU
{
public:
   CacheSetNMRU(CacheBase::cache_t cache_type,
                UInt32 index,
                UInt32 associativity,
                UInt32 threshold,
                UInt32 block_size);

   ~CacheSetNMRU() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}

using CacheSetNMRUDonuts = donuts::CacheSetNMRU;