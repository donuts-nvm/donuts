#include "cache_set_random.h"
#include "epoch_info.h"

namespace donuts
{

CacheSetRandom::CacheSetRandom(const CacheBase::cache_t cache_type,
                               const UInt32 index,
                               const UInt32 associativity,
                               const UInt32 threshold,
                               const UInt32 block_size) :
    ::CacheSet(cache_type, associativity, block_size),
    CacheSet(cache_type, index, associativity, threshold, block_size),
    ::CacheSetRandom(cache_type, associativity, block_size) {}

CacheSetRandom::~CacheSetRandom() = default;

UInt32
CacheSetRandom::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetRandom::getReplacementIndex(cntlr);

   if (countDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(CheckpointReason::CACHE_SET_THRESHOLD, m_index);

   return index;
}

}
