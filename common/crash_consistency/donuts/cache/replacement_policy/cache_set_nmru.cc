#include "cache_set_nmru.h"
#include "epoch_info.h"

namespace donuts
{

CacheSetNMRU::CacheSetNMRU(const CacheBase::cache_t cache_type,
                           const UInt32 index,
                           const UInt32 associativity,
                           const UInt32 threshold,
                           const UInt32 block_size) :
    ::CacheSet(cache_type, associativity, block_size),
    CacheSet(cache_type, index, associativity, threshold, block_size),
    ::CacheSetNMRU(cache_type, associativity, block_size) {}

CacheSetNMRU::~CacheSetNMRU() = default;

UInt32
CacheSetNMRU::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetNMRU::getReplacementIndex(cntlr);

   if (countDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(CheckpointReason::CACHE_SET_THRESHOLD, m_index);

   return index;
}

}
