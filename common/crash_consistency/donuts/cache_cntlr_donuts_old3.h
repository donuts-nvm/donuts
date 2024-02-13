//#pragma once
//
//#include "cache_cntlr_wb.h"
//#include "epoch_cntlr.h"
//
//namespace ParametricDramDirectoryMSI
//{
//
//class CacheCntlrDonuts : public CacheCntlrWrBuff
//{
//public:
//
//   typedef enum persistence_policy_t {
//      SEQUENTIAL,
//      FULLEST_FIRST,
//      BALANCED
//   } PersistencePolicy;
//
//   CacheCntlrDonuts(MemComponent::component_t mem_component,
//                    String name,
//                    core_id_t core_id,
//                    MemoryManager* memory_manager,
//                    AddressHomeLookup* tag_directory_home_lookup,
//                    Semaphore* user_thread_sem,
//                    Semaphore* network_thread_sem,
//                    UInt32 cache_block_size,
//                    CacheParameters& cache_params,
//                    ShmemPerfModel* shmem_perf_model,
//                    bool is_last_level_cache,
//                    EpochCntlr* epoch_cntlr);
//
//   ~CacheCntlrDonuts() override;
//
//   void checkpoint(CheckpointEvent::type_t event_type, UInt32 evicted_set_index) override;
//
//private:
//
//   EpochCntlr* m_epoch_cntlr;
//   PersistencePolicy m_persistence_policy;
//   std::queue<CacheBlockInfo*> m_dirty_blocks;
//
//   void periodicCommit();
//   SubsecondTime flushDirtyBlocks();
//
//   void addAllDirtyBlocks(std::queue<CacheBlockInfo*>& dirty_blocks, UInt32 set_index) const;
//   std::queue<CacheBlockInfo*> selectDirtyBlocks(UInt32 evicted_set_index) const;
//
//   SubsecondTime sendDataToDram(IntPtr address) override; // FIXME: Don't use me!
//   void sendByWriteBuffer(const WriteBufferEntry &entry) override;
//
//   void processCommit(IntPtr address, Byte* data_buf);
//   void processPersist(IntPtr address, Byte* data_buf);
//
//   void sendMsgTo(PrL1PrL2DramDirectoryMSI::ShmemMsg::msg_t msg_type, MemComponent::component_t receiver_mem_component, IntPtr address, Byte* data_buf);
//
//   // Used to periodic checkpoints
//   static SInt64 _checkpoint_timeout(UInt64 arg, UInt64 val) {
//      ((CacheCntlrDonuts *)arg)->checkpoint(CheckpointEvent::PERIODIC_TIME, 0);
//      return 0;
//   }
//
//   // Used to periodic checkpoints
//   static SInt64 _checkpoint_instr(UInt64 arg, UInt64 val) {
//      ((CacheCntlrDonuts *)arg)->checkpoint(CheckpointEvent::PERIODIC_INSTRUCTIONS, 0);
//      return 0;
//   }
//
//   static SInt64 _interrupt(UInt64 arg, UInt64 val) {
////      printf("t_now: %lu...\n", ((CacheCntlrDonuts *)arg)->getShmemPerfModel()->getElapsedTime(ShmemPerfModel::_USER_THREAD).getNS());
////      ((CacheCntlrDonuts *)arg)->periodicCommit();
//      return 0;
//   }
//
//   static PersistencePolicy getPersistencePolicy();
//
//   void printCache(); // ONLY FOR DEBUG! //
//};
//
//} // namespace ParametricDramDirectoryMSI
