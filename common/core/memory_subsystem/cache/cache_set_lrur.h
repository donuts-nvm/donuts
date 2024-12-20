#pragma once

#include "cache_set_donuts.h"
#include "cache_set_lru.h"

class CacheSetLRUR : public CacheSetDonuts, public CacheSetLRU
{
public:
   CacheSetLRUR(CacheBase::cache_t cache_type,
                UInt32 index, UInt32 associativity, UInt32 blocksize,
                CacheSetInfoLRU* set_info, UInt8 num_attempts,
                float cache_set_threshold = DEFAULT_CACHE_SET_THRESHOLD);

   ~CacheSetLRUR() override;

   UInt32 getReplacementIndex(CacheCntlr* cntlr) override;
};
