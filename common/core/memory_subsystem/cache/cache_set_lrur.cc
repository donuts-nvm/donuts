#include "cache_set_lrur.h"

CacheSetLRUR::CacheSetLRUR(CacheBase::cache_t cache_type,
                           UInt32 index,
                           UInt32 associativity,
                           UInt32 blocksize,
                           CacheSetInfoLRU *set_info,
                           UInt8 num_attempts,
                           float cache_set_threshold) :
    CacheSetLRU(cache_type, associativity, blocksize, set_info, num_attempts),
    m_index(index),
    m_cache_set_threshold(cache_set_threshold) { }

CacheSetLRUR::~CacheSetLRUR() = default;

/** 
 * Select the index to be removed.
 * 
 * Valid only for LRU_R, not LRUR_QBS.
 * For LRUR_QBS, consult getReplacementIndex method in cache_set_lru.cc
 */
UInt32
CacheSetLRUR::getReplacementIndex(CacheCntlr *cntlr)
{
   UInt32 index = 0, num_modified = 0;
   UInt8 max_bits = 0;

   // First try to find an invalid block
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      if (!m_cache_block_info_array[i]->isValid())
      {
         // Mark our newly-inserted line as most-recently used
         moveToMRU(i);
         return i;
      }

      // Find the last recently used between modified blocks
      if (m_lru_bits[i] > max_bits && isValidReplacement(i))
      {
         index = i;
         max_bits = m_lru_bits[i];
      }

      // Check if all blocks are modified
      if (m_cache_block_info_array[i]->isDirty())
         num_modified++;
   }

   auto threshold = static_cast<UInt32>(static_cast<float>(m_associativity) * m_cache_set_threshold);
   if (num_modified >= threshold)
   {
      max_bits = m_associativity - 1;
      // Return the oldest block among the modified ones
      for (UInt32 i = 0; i < m_associativity; i++)
      {
         if (m_lru_bits[i] == max_bits)
         {
            cntlr->checkpoint(CheckpointEvent::CACHE_SET_THRESHOLD, m_index);
            return i;
         }
      }
   }

   LOG_ASSERT_ERROR(index < m_associativity, "Error Finding LRU bits");

   // Mark our newly-inserted line as most-recently used
   moveToMRU(index);
   m_set_info->incrementAttempt(0);
   return index;
}

bool CacheSetLRUR::isValidReplacement(UInt32 index)
{
   CacheState::cstate_t state = m_cache_block_info_array[index]->getCState();
   return (state != CacheState::SHARED_UPGRADING && state != CacheState::MODIFIED);
}