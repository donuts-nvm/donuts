#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_plru.h"
#include "cache_set_donuts.h"

namespace donuts
{

class CacheSetPLRU final : public CacheSetDonuts, public ::CacheSetPLRU
{
public:
   CacheSetPLRU(CacheBase::cache_t cache_type,
                UInt32 index,
                UInt32 associativity,
                UInt32 threshold,
                UInt32 block_size);

   ~CacheSetPLRU() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}