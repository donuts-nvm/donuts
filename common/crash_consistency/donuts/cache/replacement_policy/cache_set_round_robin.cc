#include "cache_set_round_robin.h"

namespace donuts
{

CacheSetRoundRobin::CacheSetRoundRobin(const CacheBase::cache_t cache_type,
                                       const UInt32 index,
                                       const UInt32 associativity,
                                       const UInt32 threshold,
                                       const UInt32 block_size) :
    CacheSet(cache_type, associativity, block_size),
    CacheSetDonuts(cache_type, index, associativity, threshold, block_size),
    ::CacheSetRoundRobin(cache_type, associativity, block_size) {}

CacheSetRoundRobin::~CacheSetRoundRobin() = default;

UInt32
CacheSetRoundRobin::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetRoundRobin::getReplacementIndex(cntlr);

   if (getAmountDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(m_index);

   return index;
}

}
