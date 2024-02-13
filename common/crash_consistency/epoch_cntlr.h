#ifndef EPOCH_CNTLR_H
#define EPOCH_CNTLR_H

#include "simulator.h"
#include "core_manager.h"
#include "performance_model.h"
#include "stats.h"

class VersionedDomain
{
public:

   explicit VersionedDomain(UInt32 id);
   ~VersionedDomain();

   void increment() { m_eid++; }

   void synchronize(UInt64 remote_eid);

   [[nodiscard]] UInt32 getID() const { return m_id; }
   [[nodiscard]] UInt64 getEpochID() const { return m_eid; }

private:

   const UInt32 m_id;
   UInt64 m_eid;
};

class EpochManager;

class EpochCntlr
{
public:

   // struct VersionedDomain
   // {
   //    VersionedDomain(const UInt32 id) : m_id(id) {}
   //    ~VersionedDomain() = default;

   //    const UInt32 m_id;
   //    UInt64 m_eid;
   // };

   /**
    * @brief Construct a new Epoch Cntlr
    */
   EpochCntlr(EpochManager* epoch_manager, UInt32 vd_id, std::vector<core_id_t> cores);

   /**
    * @brief Destroy the Epoch Cntlr
    */
   ~EpochCntlr();

   /**
    * @brief Increment the current epoch
    */
   void newEpoch();

   /**
    * @brief Commit the current epoch
    */
   void commit();

   /**
    * @brief Register an Epoch ID with the last persisted data
    *
    * @param persisted_eid
    */
   void registerPersistedEID(UInt64 persisted_eid);

   [[nodiscard]] UInt64 getCurrentEID() const { return m_vd.getEpochID(); }

   [[nodiscard]] const VersionedDomain* getVersionedDomain() const { return &m_vd; }

private:

   EpochManager* m_epoch_manager;
   VersionedDomain m_vd;
   std::vector<core_id_t> m_cores;

   UInt64 getInstructionCount();
};

#endif /* EPOCH_CNTLR_H */