#pragma once

#include "../common/core/memory_subsystem/cache/cache_set_srrip.h"
#include "cache_set.h"

namespace donuts
{

class CacheSetSRRIP final : public CacheSet, public ::CacheSetSRRIP
{
public:
   CacheSetSRRIP(CacheBase::cache_t cache_type,
                 UInt32 index,
                 UInt32 associativity,
                 UInt32 threshold,
                 UInt32 block_size,
                 CacheSetInfoLRU* set_info,
                 UInt8 num_attempts,
                 UInt8 num_bits_rrip);

   ~CacheSetSRRIP() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};

}
