#ifndef EPOCH_MANAGER_H
#define EPOCH_MANAGER_H

#include "epoch_cntlr.h"
#include "simulator.h"
#include "core_manager.h"
#include "performance_model.h"
#include "hooks_manager.h"
#include "config.hpp"
#include "stats.h"

#include <cmath>

class EpochManager
{
public:

   /**
    * @brief Construct a new Epoch Manager
    */
   EpochManager();

   /**
    * @brief Destroy the Epoch Manager
    */
   ~EpochManager();

   /**
    * @brief Commit the current epoch
    */
   void commit();

   /**
    * @brief Commit the current epoch
    *
    * @param event_type
    */
   void commit(CheckpointEvent::type_t event_type);

   /**
    * @brief Register an Epoch ID with the last persisted data
    *
    * @param persisted_eid
    */
   void registerPersistedEID(UInt64 persisted_eid);

   /**
    * @brief Get the System EpochID
    * @return UInt64
    */
   [[nodiscard]] UInt64 getSystemEID() const { return m_current.eid; }

   /**
    * @brief Get PC from the Current Epoch
    * @return UInt64
    */
   [[nodiscard]] UInt64 getSystemPC() const { return m_current.pc; }

   /**
    * @brief Get the Commited EpochID
    * @return UInt64
    */
   [[nodiscard]] UInt64 getCommitedEID() const { return m_commited.eid; }

   /**
    * @brief Get the Commited PC
    * @return UInt64
    */
   [[nodiscard]] UInt64 getCommitedPC() const { return m_commited.pc; }

   /**
    * @brief Get the Commited Time
    * @return SubsecondTime
    */
   [[nodiscard]] SubsecondTime getCommitedTime() const { return m_commited.time; }

   /**
    * @brief Get the Commited Instruction
    * @return UInt64
    */
   [[nodiscard]] UInt64 getCommitedInstruction() const { return m_commited.instr; }

   /**
    * @brief Get the Persisted EpochID
    * @return UInt64
    */
   [[nodiscard]] UInt64 getPersistedEID() const { return m_persisted.eid; }

   /**
    * @brief Get the Persisted Time
    * @return SubsecondTime
    */
   [[nodiscard]] SubsecondTime getPersistedTime() const { return m_persisted.time; }

   /**
    * @brief Get the Persisted Instruction
    * @return UInt64
    */
   [[nodiscard]] UInt64 getPersistedInstruction() const { return m_commited.instr; }

   /**
    * @brief Get the Epoch Cntlr
    *
    * @param core_id
    * @return EpochCntlr*
    */
   EpochCntlr* getEpochCntlr(core_id_t core_id);

   /**
    * @brief Get the Global SystemEID
    * @return UInt64
    */
   static UInt64 getGlobalSystemEID();

   /**
    * @brief Get the single instance of the Epoch Manager
    * @return EpochManager*
    */
   static EpochManager *getInstance();

private:
   struct epoch_instant_t
   {
      epoch_instant_t() : eid(0), instr(0), pc(0), time(SubsecondTime::Zero()) {}
      UInt64 eid;
      UInt64 instr;
      IntPtr pc;
      SubsecondTime time;
   };

   FILE *m_log_file;
   std::map<IntPtr, CheckpointEvent> m_epoch_events;

   struct epoch_instant_t m_current;
   struct epoch_instant_t m_commited;
   struct epoch_instant_t m_persisted;

   SubsecondTime m_max_interval_time;
   UInt64 m_max_interval_instr;

   std::vector<EpochCntlr*> m_cntlrs;
   UInt32 m_cores_by_vd;

   void createEpochCntlrs();

   /**
    * @brief Start the epoch manager system
    * This method is called on application start
    */
   void start();

   /**
    * @brief Exit the epoch manager system
    * This method is called on application start
    */
   void exit() const;

   /**
    * @brief Interrupt the epoch manager system
    * This method is called on HOOK_PERIODIC intervals to trigger EPOCH_TIMEOUT events
    */
   void interrupt();

   static SInt64 _start(UInt64 arg, UInt64 val) { ((EpochManager *)arg)->start(); return 0; }
   static SInt64 _exit(UInt64 arg, UInt64 val) { ((EpochManager *)arg)->exit(); return 0; }
   static SInt64 _interrupt(UInt64 arg, UInt64 val) { ((EpochManager *)arg)->interrupt(); return 0; }

   // FIX-ME: Used only for test
   static SInt64 _commit(UInt64 arg, UInt64 val) { ((EpochManager *)arg)->commit(); return 0; }

   static UInt64 getMaxIntervalTime();
   static UInt64 getMaxIntervalInstructions();
   static UInt32 getNumVersionedDomains();
   static UInt32 getSharedCoresByVD();

   static UInt64 getTotalInstructionCount();

   template <typename T>
   static T gapBetweenCheckpoints(T current, T last);
};

#endif /* EPOCH_MANAGER_H */