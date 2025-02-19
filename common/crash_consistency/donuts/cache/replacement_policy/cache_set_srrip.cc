#include "cache_set_srrip.h"
#include "epoch_info.h"

namespace donuts
{

CacheSetSRRIP::CacheSetSRRIP(const CacheBase::cache_t cache_type,
                             const UInt32 index,
                             const UInt32 associativity,
                             const UInt32 threshold,
                             const UInt32 block_size,
                             CacheSetInfoLRU* set_info,
                             const UInt8 num_attempts,
                             const UInt8 num_bits_rrip) :
    ::CacheSet(cache_type, associativity, block_size),
    CacheSet(cache_type, index, associativity, threshold, block_size),
    ::CacheSetSRRIP(cache_type, associativity, block_size, set_info, num_attempts, num_bits_rrip) {}

CacheSetSRRIP::~CacheSetSRRIP() = default;

UInt32
CacheSetSRRIP::getReplacementIndex(CacheCntlr* cntlr)
{
   const auto index = ::CacheSetSRRIP::getReplacementIndex(cntlr);

   if (countDirtyBlocks() >= m_threshold)
      cntlr->checkpoint(CheckpointReason::CACHE_SET_THRESHOLD, m_index);

   return index;
}

}
