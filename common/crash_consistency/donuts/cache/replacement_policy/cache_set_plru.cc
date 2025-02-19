#include "cache_set_plru.h"
#include "epoch_info.h"

namespace donuts
{

CacheSetPLRU::CacheSetPLRU(const CacheBase::cache_t cache_type,
                           const UInt32 index,
                           const UInt32 associativity,
                           const UInt32 threshold,
                           const UInt32 block_size) :
    ::CacheSet(cache_type, associativity, block_size),
    CacheSet(cache_type, index, associativity, threshold, block_size),
    ::CacheSetPLRU(cache_type, associativity, block_size) {}

CacheSetPLRU::~CacheSetPLRU() = default;

UInt32
CacheSetPLRU::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetPLRU::getReplacementIndex(cntlr);

   if (countDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(CheckpointReason::CACHE_SET_THRESHOLD, m_index);

   return index;
}

}
