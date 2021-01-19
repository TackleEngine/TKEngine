#include "TKSynchronize.h"
USING_TK_NAMESPACE
TKCriticalSection g_SafeCS;

unsigned int TKSynchronize::WaitAll(TKSynchronize** pSynchronize, unsigned int uiNum, bool bWaitAll, DWORD dwMilliseconds)
{
    TKMAC_ASSERT(uiNum >= 1&& uiNum <= MAXIMUM_WAIT_OBJECTS);
    static HANDLE handle[MAXIMUM_WAIT_OBJECTS];
    for (unsigned int i = 0; i < uiNum; i++)
    {
        handle[i] = (HANDLE)pSynchronize[i]->GetHandle();
    }
    /*
		* bWaitAll 等待的类型，如果为TRUE，表示除非对象都发出信号，否则就一直等待下去；如果FALSE，表示任何对象发出信号即可
	 */
    DWORD dw = WaitForMultipleObjects(uiNum, handle, bWaitAll, dwMilliseconds);
    switch (dw)
    {
        case WAIT_FAILED:			// 函数执行失败
            return WF_FAILED;
        case WAIT_TIMEOUT:			// 对象保持未发信号的状态，但规定的等待超时时间已经超过
            return WF_TIMEOUT;
        case WAIT_OBJECT_0:			// 所有对象都发出信号
            return WF_OBJECT0;
    }
    return WF_FAILED;
}

void TKSynchronize::TKSafeOutPutDebugString(const TCHAR * pcString, ...)
{
    g_SafeCS.Lock();
    char *pArgs;
    pArgs = (char*)&pcString + sizeof(pcString);
    _vstprintf_s(TKSystem::ms_sLogBuffer, LOG_BUFFER_SIZE, pcString, pArgs);
    OutputDebugString(TKSystem::ms_sLogBuffer);
    g_SafeCS.Unlock();
}

//==================================Semaphore Begin==========================
TKSemaphore::TKSemaphore(unsigned int uiCount, unsigned int MaxCount)
{
    TKMAC_ASSERT(uiCount <= MaxCount);
    Semaphore = CreateSemaphore(NULL, uiCount, MaxCount, NULL);
    m_uiMaxCount = MaxCount;
    TKMAC_ASSERT(Semaphore);
}

TKSemaphore::~TKSemaphore()
{
    BOOL closed = CloseHandle((HANDLE)Semaphore);
    Semaphore = NULL;
    TKMAC_ASSERT(closed);
}

void TKSemaphore::Enter()
{
    DWORD result = WaitForSingleObject((HANDLE)Semaphore, INFINITE);
    TKMAC_ASSERT(result);
    // result:
    //   WAIT_ABANDONED (0x00000080)
    //   WAIT_OBJECT_0  (0x00000000), signaled
    //   WAIT_TIMEOUT   (0x00000102), [not possible with INFINITE]
    //   WAIT_FAILED    (0xFFFFFFFF), not signaled
}

void TKSemaphore::Leave(unsigned int uiReleaseCount)
{
    BOOL released = ReleaseSemaphore((HANDLE)Semaphore, uiReleaseCount, NULL);
    TKMAC_ASSERT(released);
}
//==================================Semaphore End============================

//==================================Mutex Begin============================
TKMutex::TKMutex()
{
    m_Mutex = CreateMutex(NULL, FALSE, NULL);
    TKMAC_ASSERT(m_Mutex);
}

TKMutex::~TKMutex()
{
    BOOL closed = CloseHandle((HANDLE)m_Mutex);
    m_Mutex = NULL;
    TKMAC_ASSERT(closed);
}

void TKMutex::Enter()
{
    DWORD result = WaitForSingleObject((HANDLE)m_Mutex, INFINITE);
    TKMAC_ASSERT(result != WAIT_FAILED);
    // result:
    //   WAIT_ABANDONED (0x00000080)
    //   WAIT_OBJECT_0  (0x00000000), signaled
    //   WAIT_TIMEOUT   (0x00000102), [not possible with INFINITE]
    //   WAIT_FAILED    (0xFFFFFFFF), not signaled
}

void TKMutex::Leave()
{
    BOOL released = ReleaseMutex((HANDLE)m_Mutex);
    TKMAC_ASSERT(released);
}
//==================================Mutex End============================

TKEvent::TKEvent(void)
{
    Event = NULL;
}

/**
* Cleans up the event handle if valid
*/
TKEvent::~TKEvent(void)
{
    if (Event != NULL)
    {
        CloseHandle(Event);
    }
}

/**
* Waits for the event to be signaled before returning
*/
void TKEvent::Lock(void)
{
    WaitForSingleObject(Event, INFINITE);
}

/**
* Triggers the event so any waiting threads are allowed access
*/
void TKEvent::Unlock(void)
{
    PulseEvent(Event);
}

/**
* Creates the event. Manually reset events stay triggered until reset.
* Named events share the same underlying event.
*
* @param bIsManualReset Whether the event requires manual reseting or not
* @param InName Whether to use a commonly shared event or not. If so this
* is the name of the event to share.
*
* @return Returns TRUE if the event was created, FALSE otherwise
*/
bool TKEvent::Create(bool bIsManualReset, const TCHAR* InName)
{
    // Create the event and default it to non-signaled

    Event = CreateEvent(NULL, bIsManualReset, 0, InName);
    return Event != NULL;
}

/**
* Triggers the event so any waiting threads are activated
*/
void TKEvent::Trigger(void)
{
    SetEvent(Event);
}

/**
* Resets the event to an untriggered (waitable) state
*/
void TKEvent::Reset(void)
{
    ResetEvent(Event);
}

/**
* Triggers the event and resets the triggered state NOTE: This behaves
* differently for auto-reset versus manual reset events. All threads
* are released for manual reset events and only one is for auto reset
*/
void TKEvent::Pulse(void)
{
    PulseEvent(Event);
}

/**
* Waits for the event to be triggered
*
* @param WaitTime Time in milliseconds to wait before abandoning the event
* (DWORD)-1 is treated as wait infinite
*
* @return TRUE if the event was signaled, FALSE if the wait timed out
*/
bool TKEvent::Wait(DWORD WaitTime)
{
    return WaitForSingleObject(Event, WaitTime) == WAIT_OBJECT_0;
}
bool TKEvent::IsTrigger()
{
    return Wait(0);
}
