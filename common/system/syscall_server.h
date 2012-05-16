#ifndef SYSCALL_SERVER_H
#define SYSCALL_SERVER_H

#include "fixed_types.h"
#include "subsecond_time.h"

#include <iostream>
#include <unordered_map>
#include <list>

// -- For futexes --
#include <linux/futex.h>
#include <sys/time.h>
#include <errno.h>

class Core;

// -- Special Class to Handle Futexes
class SimFutex
{
   private:
      struct Waiter
      {
         Waiter(thread_id_t _thread_id, int _mask, SubsecondTime _timeout)
            : thread_id(_thread_id), mask(_mask), timeout(_timeout)
            {}
         thread_id_t thread_id;
         int mask;
         SubsecondTime timeout;
      };
      typedef std::list<Waiter> ThreadQueue;
      ThreadQueue m_waiting;

   public:
      SimFutex();
      ~SimFutex();
      bool enqueueWaiter(thread_id_t thread_id, int mask, SubsecondTime time, SubsecondTime timeout_time, SubsecondTime &time_end);
      thread_id_t dequeueWaiter(thread_id_t thread_by, int mask, SubsecondTime time);
      thread_id_t requeueWaiter(SimFutex *requeue_futex);
      void wakeTimedOut(SubsecondTime time);
};

class SyscallServer
{
   public:
      struct futex_args_t {
         int *uaddr;
         int op;
         int val;
         const struct timespec *timeout;
         int val2;
         int *uaddr2;
         int val3;
      };

      SyscallServer();
      ~SyscallServer();

      IntPtr handleFutexCall(thread_id_t thread_id, futex_args_t &args, SubsecondTime curr_time, SubsecondTime &end_time);

   private:
      // Handling Futexes
      IntPtr futexWait(thread_id_t thread_id, int *uaddr, int val, int act_val, int val3, SubsecondTime curr_time, SubsecondTime timeout_time, SubsecondTime &end_time);
      IntPtr futexWake(thread_id_t thread_id, int *uaddr, int nr_wake, int val3, SubsecondTime curr_time, SubsecondTime &end_time);
      IntPtr futexWakeOp(thread_id_t thread_id, int op, int *uaddr, int val, int *uaddr2, int nr_wake, int nr_wake2, SubsecondTime curr_time, SubsecondTime &end_time);
      IntPtr futexCmpRequeue(thread_id_t thread_id, int *uaddr, int val, int *uaddr2, int val3, int act_val, SubsecondTime curr_time, SubsecondTime &end_time);

      thread_id_t wakeFutexOne(SimFutex *sim_futex, thread_id_t thread_by, int mask, SubsecondTime curr_time);
      int futexDoOp(Core *core, int op, int *uaddr);

      void futexPeriodic(SubsecondTime time);

      static void hook_periodic(SyscallServer* ptr, subsecond_time_t time)
      { ptr->futexPeriodic(time); }

   private:
      SubsecondTime m_reschedule_cost;

      // Handling Futexes
      typedef std::unordered_map<IntPtr, SimFutex> FutexMap;
      FutexMap m_futexes;
};

#endif
