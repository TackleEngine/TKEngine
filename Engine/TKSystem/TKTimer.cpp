
#include "TKTimer.h"
#include "TKSystem.h"

USING_TK_NAMESPACE

TKTimer* TKTimer::ms_pTimer = nullptr;
TKTimer::TKTimer()
{
    InitGameTime();
    ms_pTimer = this;
}

TKTimer::~TKTimer()
{
}

void TKTimer::InitGameTime()
{
    m_iFrameCount = 0;
    m_fFPS = 0;
    m_fTime = 0;
    m_fLastFPSTime = 0;
    m_fTimeSlice = 0;
    m_fLastTime = 0;
    m_fDetTime = 0;

    //返回硬件支持的高精度计数器的频率, 非零，硬件支持高精度计数器；零，硬件不支持，读取失败
    if (QueryPerformanceFrequency((LARGE_INTEGER*)&m_int64OneSecondTicks)) {
        m_bUseLargeTime = true;
        QueryPerformanceCounter((LARGE_INTEGER*)&m_int64TimeTickStartCounts);
    }
    else
    {
        m_bUseLargeTime = false;
        m_ulTimeStart = timeGetTime();
    }
}

int TKTimer::GetRandSeed()
{
    return ((LARGE_INTEGER*)&m_int64TimeTickStartCounts)->LowPart;
}

double TKTimer::GetGamePlayTime()
{
    __int64 m_int64TimeCurrenCounts;
    if (m_bUseLargeTime)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&m_int64TimeCurrenCounts);
        return  ((m_int64TimeCurrenCounts - m_int64TimeTickStartCounts)*(1.0 / m_int64OneSecondTicks)*1000.0);
    }
    else
    {
        return (timeGetTime() - m_ulTimeStart);
    }
}

void TKTimer::UpdateFPS()
{
    m_fTime = GetGamePlayTime() * 0.001;
    m_fDetTime = m_fTime - m_fLastTime;
    m_fLastTime = m_fTime;
    if (m_fTime - m_fLastTime > 1.0f)
    {
        m_fLastFPSTime = m_fTime;
        m_fFPS = m_iFrameCount;
        m_iFrameCount = 0;
    }
    else
    {
        m_iFrameCount++;
    }
}



