#include "cache_set_mru.h"

namespace donuts
{

CacheSetMRU::CacheSetMRU(const CacheBase::cache_t cache_type,
                         const UInt32 index,
                         const UInt32 associativity,
                         const UInt32 threshold,
                         const UInt32 block_size) :
    CacheSet(cache_type, associativity, block_size),
    CacheSetDonuts(cache_type, index, associativity, threshold, block_size),
    ::CacheSetMRU(cache_type, associativity, block_size) {}

CacheSetMRU::~CacheSetMRU() = default;

UInt32
CacheSetMRU::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetMRU::getReplacementIndex(cntlr);

   if (getAmountDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(m_index);

   return index;
}

}
