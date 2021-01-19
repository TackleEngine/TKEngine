#ifndef CETHREAD_H
#define CETHREAD_H

#include "TKSystem.h"
#include "TKSynchronize.h"

TK_NAMESPACE_BEGIN
/**
@class	MAY::TKThread
@brief	多线线程基类.
字类实现Run()方法, 通过IsStopRequested()来判断线程是否结束.
如果需要在VS调试时显示线程名, 请在子类中重载GetThreadName()方法.
*/
class TKSYSTEM_API TKThread
	{
        public:
			/* 
			* 优先级
			*/
			enum Priority
			{
				Low,
				Normal,
				High,
			};
			/*
			* 线程状态
			*/
			enum ThreadState
			{
				TS_START,
				TS_SUSPEND,
				TS_STOP,
			};
        public:
			TKThread();
			virtual ~TKThread();
			/*
			* 设置线程优先级
			*/
			void SetPriority(Priority p);
			/*
			* 获取线程优先级级别
			*/
			unsigned int GetPriority() const
			{
				return m_priority;
			}
			void SetStackSize(unsigned int uiSize);
			unsigned int GetStackSize() const
			{
				return m_stackSize;
			}

			void Start();
			void Suspend();
			bool IsRunning() const;
			void Sleep(DWORD dwMillseconds);
			bool IsStopTrigger();
			void Stop();

			static void SetThreadName(const char* name)
			{
			}
			FORCEINLINE ThreadState GetThreadState()
			{
				return m_ThreadState;
			}
        protected:
			virtual void Run() = 0;
			virtual const TCHAR* GetThreadName();
			private:
			static DWORD THREAD_CALLBACK ThreadProc(void* t);
			private:
			void* m_hThread;
			Priority m_priority;
			unsigned int m_stackSize;
			protected:
			ThreadState m_ThreadState;
			TKEvent m_StopEvent;
};

TK_NAMESPACE_END

#endif
