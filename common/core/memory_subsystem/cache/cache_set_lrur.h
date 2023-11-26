#ifndef CACHE_SET_LRUR_H
#define CACHE_SET_LRUR_H

#include "cache_set_lru.h"

class CacheSetLRUR : public CacheSetLRU
{
public:
   static const constexpr float DEFAULT_CACHE_SET_THRESHOLD = 1.0;

   CacheSetLRUR(CacheBase::cache_t cache_type, UInt32 associativity, UInt32 blocksize,
                CacheSetInfoLRU *set_info, UInt8 num_attempts,
                float cache_set_threshold = DEFAULT_CACHE_SET_THRESHOLD);

   ~CacheSetLRUR() override;

   UInt32 getReplacementIndex(CacheCntlr *cntlr) override;

protected:

   float m_cache_set_threshold;

   bool isValidReplacement(UInt32 index) override;
};

#endif /* CACHE_SET_LRUR_H */