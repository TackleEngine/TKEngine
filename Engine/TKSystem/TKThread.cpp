
#include "TKThread.h"

USING_TK_NAMESPACE

DWORD THREAD_CALLBACK TKThread::ThreadProc(void* t)
{
    TKThread* pThread = (TKThread*)(t);
    SetThreadName(pThread->GetThreadName());
    pThread->Run();
    return 0;
}

TKThread::TKThread()
        :m_hThread(nullptr)
        ,m_priority(Normal)
        ,m_stackSize(0)
{
    TKMAC_ASSERT(!IsRunning());
    m_hThread = ::CreateThread(0, m_stackSize, ThreadProc, this, CREATE_SUSPENDED, nullptr);
    TKMAC_ASSERT(m_hThread);
    m_ThreadState = TS_SUSPEND;
    SetPriority(m_priority);
    m_StopEvent.Create(true);
    m_StopEvent.Reset();
}

TKThread::~TKThread()
{
    if (IsRunning())
    {
        // force to exit
        TerminateThread(m_hThread, 0);
    }
}

void TKThread::SetPriority(Priority p)
{
    int nPriority = THREAD_PRIORITY_NORMAL;
    if (p == Low)
        nPriority = THREAD_PRIORITY_BELOW_NORMAL;
    else if (p == Normal)
        nPriority = THREAD_PRIORITY_NORMAL;
    else if (p == High)
        nPriority = THREAD_PRIORITY_ABOVE_NORMAL;

    ::SetThreadPriority(m_hThread, nPriority);
}

void TKThread::SetStackSize(unsigned int uiSize)
{
}

void TKThread::Start()
{
    if (m_ThreadState == TS_SUSPEND)
    {
        ResumeThread((HANDLE)m_hThread);
        m_ThreadState = TS_START;
    }
}

void TKThread::Suspend()
{
    if (m_ThreadState == TS_START)
    {
        SuspendThread((HANDLE)m_hThread);
        m_ThreadState = TS_SUSPEND;
    }
}

bool TKThread::IsRunning() const
{
    if (NULL != m_hThread)
    {
        DWORD exitCode = 0;
        if (GetExitCodeThread(m_hThread, &exitCode))
        {
            if (STILL_ACTIVE == exitCode)
            {
                return true;
            }
        }
    }
    return false;
}

void TKThread::Sleep(DWORD dwMillseconds)
{
    if (m_ThreadState == TS_START)
    {
        ::Sleep(dwMillseconds);
    }
}

bool TKThread::IsStopTrigger()
{
    return m_StopEvent.IsTrigger();
}

void TKThread::Stop()
{
    if (m_ThreadState == TS_START)
    {
        TKMAC_ASSERT(this->IsRunning());
        TKMAC_ASSERT(NULL != m_hThread);

        m_StopEvent.Trigger();
        m_ThreadState = TS_STOP;
        // wait for the thread to terminate
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}



