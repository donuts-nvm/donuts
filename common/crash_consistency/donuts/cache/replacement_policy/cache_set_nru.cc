#include "cache_set_nru.h"
#include "epoch_info.h"

namespace donuts
{

CacheSetNRU::CacheSetNRU(const CacheBase::cache_t cache_type,
                         const UInt32 index,
                         const UInt32 associativity,
                         const UInt32 threshold,
                         const UInt32 block_size) :
    ::CacheSet(cache_type, associativity, block_size),
    CacheSet(cache_type, index, associativity, threshold, block_size),
    ::CacheSetNRU(cache_type, associativity, block_size) {}

CacheSetNRU::~CacheSetNRU() = default;

UInt32
CacheSetNRU::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetNRU::getReplacementIndex(cntlr);

   if (countDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(CheckpointReason::CACHE_SET_THRESHOLD, m_index);

   return index;
}

}
