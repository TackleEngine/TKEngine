
/*
  * 包含线程同步的类
*/
#ifndef TK_SYNCHRONIZE_H
#define TK_SYNCHRONIZE_H
#include "TKSystem.h"
#include "TKUtils.h"
TK_NAMESPACE_BEGIN

/*
  * 临界区
*/
	class TKSYSTEM_API TKCriticalSection
    {
		/**
		* The windows specific critical section
		*/
		CRITICAL_SECTION CriticalSection;

    public:
        /**
        * Constructor that initializes the aggregated critical section
        */
        FORCEINLINE TKCriticalSection(void)
        {
            InitializeCriticalSection(&CriticalSection);
            (&CriticalSection, 4000);
        }

        /**
        * Destructor cleaning up the critical section
        */
        FORCEINLINE ~TKCriticalSection(void)
        {
            DeleteCriticalSection(&CriticalSection);
        }

        /**
        * Locks the critical section
        */
        FORCEINLINE void Lock(void)
        {
            // Spin first before entering critical section, causing ring-0 transition and context switch.
            EnterCriticalSection(&CriticalSection);
        }

        /**
        * Releases the lock on the critical seciton
        */
        FORCEINLINE void Unlock(void)
        {
            LeaveCriticalSection(&CriticalSection);
        }
    };

/*
  * 线程同步
*/
    class TKSYSTEM_API TKSynchronize
    {
    public:
        TKSynchronize()
        {

        }
        virtual ~TKSynchronize()
        {

        }
        virtual void* GetHandle() = 0;
        enum
        {
            WF_OBJECT0 = 0,
            WF_TIMEOUT = 256,
            WF_FAILED = 0xFFFFFFFF
        };
        static unsigned int WaitAll(TKSynchronize** pSynchronize, unsigned int uiNum, bool bWaitAll, DWORD dwMilliseconds = (DWORD)-1);
        static void TKSafeOutPutDebugString(const TCHAR* pcString, ...);
    };

/*
	*信号量
*/
    class TKSYSTEM_API TKSemaphore : public TKSynchronize
    {
    public:
        TKSemaphore(unsigned int uiCount, unsigned int MaxCount);
        virtual ~TKSemaphore();

        virtual void Enter();
        virtual void Leave(unsigned int uiReleaseCount);
        virtual void* GetHandle()
        {
            return Semaphore;
        }

    protected:
        void* Semaphore;
        unsigned int m_uiMaxCount;
    };

/*
	* 互斥量
*/
    class TKSYSTEM_API TKMutex : public TKSynchronize
    {
    public:
        TKMutex();
        virtual ~TKMutex();

        virtual void Enter();
        virtual void Leave();
        virtual void* GetHandle()
        {
            return m_Mutex;
        }

    protected:
        void* m_Mutex;
    };

    class TKSYSTEM_API TKEvent : public TKSynchronize
    {
        /**
        * The handle to the event
        */
        HANDLE Event;

    public:
        virtual void * GetHandle()
        {
            return Event;
        }
        /**
        * Constructor that zeroes the handle
        */
        TKEvent(void);

        /**
        * Cleans up the event handle if valid
        */
        virtual ~TKEvent(void);

        /**
        * Waits for the event to be signaled before returning
        */
        virtual void Lock(void);

        /**
        * Triggers the event so any waiting threads are allowed access
        */
        virtual void Unlock(void);

        /**
        * Named events share the same underlying event.
        *
        * @param bIsManualReset Whether the event requires manual reseting or not
        * @param InName Whether to use a commonly shared event or not. If so this
        * is the name of the event to share.
        *
        * @return Returns TRUE if the event was created, FALSE otherwise
        */
        virtual bool Create(bool bIsManualReset = FALSE, const TCHAR* InName = NULL);

        /**
        * Triggers the event so any waiting threads are activated
        */
        virtual void Trigger(void);

        /**
        * Resets the event to an untriggered (waitable) state
        */
        virtual void Reset(void);

        /**
        * Triggers the event and resets the triggered state NOTE: This behaves
        * differently for auto-reset versus manual reset events. All threads
        * are released for manual reset events and only one is for auto reset
        */
        virtual void Pulse(void);

        /**
        * Waits for the event to be triggered
        *
        * @param WaitTime Time in milliseconds to wait before abandoning the event
        * (DWORD)-1 is treated as wait infinite
        *
        * @return TRUE if the event was signaled, FALSE if the wait timed out
        */
        virtual bool Wait(DWORD WaitTime = (DWORD)-1);

        virtual bool IsTrigger();
    };


TK_NAMESPACE_END

#endif // !TK_SYNCHRONIZE_H
