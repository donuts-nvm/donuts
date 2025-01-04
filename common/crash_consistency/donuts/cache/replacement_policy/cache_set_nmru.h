#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_nmru.h"
#include "cache_set_donuts.h"

namespace donuts
{

class CacheSetNMRU final : public CacheSetDonuts, public ::CacheSetNMRU
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