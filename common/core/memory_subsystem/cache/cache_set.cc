#include "cache_set.h"
#include "cache_set_lru.h"
#include "cache_set_mru.h"
#include "cache_set_nmru.h"
#include "cache_set_nru.h"
#include "cache_set_plru.h"
#include "cache_set_random.h"
#include "cache_set_round_robin.h"
#include "cache_set_srrip.h"
#include "cache_base.h"
#include "log.h"
#include "simulator.h"
#include "config.h"
#include "config.hpp"

CacheSet::CacheSet(CacheBase::cache_t cache_type,
      UInt32 associativity, UInt32 blocksize):
      m_associativity(associativity), m_blocksize(blocksize)
{
   m_cache_block_info_array = new CacheBlockInfo*[m_associativity];
   for (UInt32 i = 0; i < m_associativity; i++)
   {
      m_cache_block_info_array[i] = CacheBlockInfo::create(cache_type);
   }

   if (Sim()->getFaultinjectionManager())
   {
      m_blocks = new char[m_associativity * m_blocksize];
      memset(m_blocks, 0x00, m_associativity * m_blocksize);
   } else {
      m_blocks = NULL;
   }
}

CacheSet::~CacheSet()
{
   for (UInt32 i = 0; i < m_associativity; i++)
      delete m_cache_block_info_array[i];
   delete [] m_cache_block_info_array;
   delete [] m_blocks;
}

void
CacheSet::read_line(UInt32 line_index, UInt32 offset, Byte *out_buff, UInt32 bytes, bool update_replacement)
{
   assert(offset + bytes <= m_blocksize);
   //assert((out_buff == NULL) == (bytes == 0));

   if (out_buff != NULL && m_blocks != NULL)
      memcpy((void*) out_buff, &m_blocks[line_index * m_blocksize + offset], bytes);

   if (update_replacement)
      updateReplacementIndex(line_index);
}

void
CacheSet::write_line(UInt32 line_index, UInt32 offset, Byte *in_buff, UInt32 bytes, bool update_replacement)
{
   assert(offset + bytes <= m_blocksize);
   //assert((in_buff == NULL) == (bytes == 0));

   if (in_buff != NULL && m_blocks != NULL)
      memcpy(&m_blocks[line_index * m_blocksize + offset], (void*) in_buff, bytes);

   if (update_replacement)
      updateReplacementIndex(line_index);
}

CacheBlockInfo*
CacheSet::find(IntPtr tag, UInt32* line_index)
{
   for (SInt32 index = m_associativity-1; index >= 0; index--)
   {
      if (m_cache_block_info_array[index]->getTag() == tag)
      {
         if (line_index != NULL)
            *line_index = index;
         return (m_cache_block_info_array[index]);
      }
   }
   return NULL;
}

bool
CacheSet::invalidate(IntPtr& tag)
{
   for (SInt32 index = m_associativity-1; index >= 0; index--)
   {
      if (m_cache_block_info_array[index]->getTag() == tag)
      {
         m_cache_block_info_array[index]->invalidate();
         return true;
      }
   }
   return false;
}

void
CacheSet::insert(CacheBlockInfo* cache_block_info, Byte* fill_buff, bool* eviction, CacheBlockInfo* evict_block_info, Byte* evict_buff, CacheCntlr *cntlr)
{
   // This replacement strategy does not take into account the fact that
   // cache blocks can be voluntarily flushed or invalidated due to another write request
   const UInt32 index = getReplacementIndex(cntlr);
   assert(index < m_associativity);

   assert(eviction != NULL);

   if (m_cache_block_info_array[index]->isValid())
   {
      *eviction = true;
      // FIXME: This is a hack. I dont know if this is the best way to do
      evict_block_info->clone(m_cache_block_info_array[index]);
      if (evict_buff != NULL && m_blocks != NULL)
         memcpy((void*) evict_buff, &m_blocks[index * m_blocksize], m_blocksize);
   }
   else
   {
      *eviction = false;
   }

   // FIXME: This is a hack. I dont know if this is the best way to do
   m_cache_block_info_array[index]->clone(cache_block_info);

   if (fill_buff != NULL && m_blocks != NULL)
      memcpy(&m_blocks[index * m_blocksize], (void*) fill_buff, m_blocksize);
}

char*
CacheSet::getDataPtr(UInt32 line_index, UInt32 offset)
{
   return &m_blocks[line_index * m_blocksize + offset];
}

CacheSet*
CacheSet::createCacheSet(const String& cfgname, const core_id_t core_id,
      const CacheBase::ReplacementPolicy replacement_policy,
      const CacheBase::cache_t cache_type,
      const UInt32 associativity, const UInt32 blocksize, CacheSetInfo* set_info)
{
   return createCacheSet(replacement_policy, cache_type, associativity, blocksize, set_info,
                         getNumQBSAttempts(replacement_policy, cfgname, core_id),
                         CacheSetSRRIP::getNumBits(replacement_policy, cfgname, core_id));
}

CacheSet*
CacheSet::createCacheSet(const CacheBase::ReplacementPolicy replacement_policy,
      const CacheBase::cache_t cache_type,
      const UInt32 associativity, const UInt32 blocksize, CacheSetInfo* set_info,
      const UInt8 num_attempts, const UInt8 rrip_numbits)
{
   switch (replacement_policy)
   {
      case CacheBase::ROUND_ROBIN:
         return new CacheSetRoundRobin(cache_type, associativity, blocksize);

      case CacheBase::LRU:
      case CacheBase::LRU_QBS:
         return new CacheSetLRU(cache_type, associativity, blocksize, dynamic_cast<CacheSetInfoLRU*>(set_info), num_attempts);

      case CacheBase::NRU:
         return new CacheSetNRU(cache_type, associativity, blocksize);

      case CacheBase::MRU:
         return new CacheSetMRU(cache_type, associativity, blocksize);

      case CacheBase::NMRU:
         return new CacheSetNMRU(cache_type, associativity, blocksize);

      case CacheBase::PLRU:
         return new CacheSetPLRU(cache_type, associativity, blocksize);

      case CacheBase::SRRIP:
      case CacheBase::SRRIP_QBS:
         return new CacheSetSRRIP(cache_type, associativity, blocksize, dynamic_cast<CacheSetInfoLRU*>(set_info), num_attempts, rrip_numbits);

      case CacheBase::RANDOM:
         return new CacheSetRandom(cache_type, associativity, blocksize);

      default:
         LOG_PRINT_ERROR("Unrecognized Cache Replacement Policy: %i", replacement_policy);
   }
}

CacheSetInfo*
CacheSet::createCacheSetInfo(const String& name, const String& cfgname, const core_id_t core_id, const CacheBase::ReplacementPolicy replacement_policy, const UInt32 associativity)
{
   switch(replacement_policy)
   {
      case CacheBase::LRU:
      case CacheBase::LRU_QBS:
      case CacheBase::SRRIP:
      case CacheBase::SRRIP_QBS:
         return new CacheSetInfoLRU(name, cfgname, core_id, associativity, getNumQBSAttempts(replacement_policy, cfgname, core_id));
      default:
         return NULL;
   }
}

UInt8
CacheSet::getNumQBSAttempts(const CacheBase::ReplacementPolicy policy, const String& cfgname, const core_id_t core_id)
{
   switch(policy)
   {
      case CacheBase::LRU_QBS:
      case CacheBase::SRRIP_QBS:
         return Sim()->getCfg()->getIntArray(cfgname + "/qbs/attempts", core_id);
      default:
         return 1;
   }
}

CacheBase::ReplacementPolicy
CacheSet::parsePolicyType(const String& policy)
{
   if (policy == "round_robin")
      return CacheBase::ROUND_ROBIN;
   if (policy == "lru")
      return CacheBase::LRU;
   if (policy == "lru_qbs")
      return CacheBase::LRU_QBS;
   if (policy == "nru")
      return CacheBase::NRU;
   if (policy == "mru")
      return CacheBase::MRU;
   if (policy == "nmru")
      return CacheBase::NMRU;
   if (policy == "plru")
      return CacheBase::PLRU;
   if (policy == "srrip")
      return CacheBase::SRRIP;
   if (policy == "srrip_qbs")
      return CacheBase::SRRIP_QBS;
   if (policy == "random")
      return CacheBase::RANDOM;

   LOG_PRINT_ERROR("Unknown replacement policy %s", policy.c_str());
}

bool CacheSet::isValidReplacement(UInt32 index)
{
   if (m_cache_block_info_array[index]->getCState() == CacheState::SHARED_UPGRADING)
   {
      return false;
   }
   else
   {
      return true;
   }
}
