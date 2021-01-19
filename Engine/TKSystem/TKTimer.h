#ifndef TK_TIMER_H
#define TK_TIMER_H
#include "TKSystem.h"

TK_NAMESPACE_BEGIN

class TKSYSTEM_API TKTimer
       {
        public:
			TKTimer();
			~TKTimer();

			double GetGamePlayTime();		//获取当前已进行的时间,单位毫秒
			void UpdateFPS();
			FORCEINLINE double GetFPS() { return m_fFPS; }
			static TKTimer* ms_pTimer;
			double GetDetTime() { return m_fDetTime; }
			int GetRandSeed();

			private:
			bool m_bUseLargeTime;					//使用大时间标志
			__int64 m_int64OneSecondTicks;			//一秒内的滴答次数
			__int64 m_int64TimeTickStartCounts;		//开始的滴答计数值
			unsigned long m_ulTimeStart;			//timeGetTime开始时间
			int m_iFrameCount;
			double m_fFPS;
			double m_fTime, m_fLastFPSTime, m_fTimeSlice;
			double m_fDetTime, m_fLastTime;
			void InitGameTime();
        };

TK_NAMESPACE_END

#endif // TK_TIMER_H

