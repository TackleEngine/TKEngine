/*
  * 封装了一些常用函数
*/
#ifndef TK_SYSTEM_H
#define TK_SYSTEM_H

#include "TKPlatform.h"
#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <memory.h>
#include <sys/stat.h>
#include <atlsimpstr.h>

//去除模板导出编译的警告
// #pragma warning(disable:4595)

TK_NAMESPACE_BEGIN

/*
	描述:
	_stdcall、__cdecl和__fastcall是三种函数调用协议,函数调用协议会影响函数参数的入栈方式、栈内数据的清除方式、编译器函数名的修饰规则等;
	1.调用协议常用场合:
		__stdcall：Windows API默认的函数调用协议;
		__cdecl：C/C++默认的函数调用协议;
		__fastcall：适用于对性能要求较高的场合;
	2.函数参数入栈方式:
		__stdcall：函数参数由右向左入栈;
		_cdecl：函数参数由右向左入栈;
		__fastcall：从左开始不大于4字节的参数放入CPU的ECX和EDX寄存器，其余参数从右向左入栈;
	3.栈内数据清除方式:
		__stdcall：函数调用结束后由被调用函数清除栈内数据;
		__cdecl：函数调用结束后由函数调用者清除栈内数据;
		__fastcall：函数调用结束后由被调用函数清除栈内数据;
*/
#define THREAD_CALLBACK	__stdcall

#define LOG_BUFFER_SIZE 65535
class TKSYSTEM_API TKSystem
{
public:
	static TCHAR ms_sLogBuffer[LOG_BUFFER_SIZE];
	static DWORD ms_dwMainThreadID;
};

//内存拷贝函数
FORCEINLINE bool TKMemcpy(void* pDest, const void* pSrc, unsigned int uiCountSize, unsigned int uiDestBufferSize = 0)
{
	if (!uiDestBufferSize)
	{
		uiDestBufferSize = uiCountSize;
	}

#if TK_PLATFORM == TK_PLATFORM_WIN
	return (memcpy_s(pDest, uiDestBufferSize, pSrc, uiCountSize) == 0);
#else
	return false;
#endif
}

//实现原子性的加减
FORCEINLINE void TKLockIncrement(long* pRefCcount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_InterlockedIncrement(pRefCcount);
#else
	return;
#endif
}

//实现原子性的加减
FORCEINLINE void TKLockedDecrement(long * pRefCount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_InterlockedDecrement(pRefCount);
#else
	return;
#endif
}

//内存初始化
FORCEINLINE void TKMemset(void *pDest, int iVal, unsigned int uiCount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	memset(pDest, iVal, uiCount);
#else
	return;
#endif
}

//获取Unicode字符串长度
FORCEINLINE unsigned int TKStrLen(const TCHAR* pStr)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return (unsigned int)_tcslen(pStr);
#else
	return 0;
#endif
}

//字符串拼接
FORCEINLINE void TKStrcat(TCHAR * pDest, unsigned int uiCount, const TCHAR * pSource)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_tcscat_s(pDest, uiCount, pSource);
#else
	return;
#endif
}

//Find the next token in a string.
FORCEINLINE TCHAR * TKStrtok(TCHAR *strToken, const TCHAR *strDelimit, TCHAR ** pContext)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return _tcstok_s(strToken, strDelimit, pContext);
#else
	return NULL;
#endif
}

//Scan a string for the last occurrence of a character.
FORCEINLINE const TCHAR * TKCsrchr(const TCHAR *pString, int c)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return _tcsrchr(pString, c);
#else
	return NULL;
#endif
}

//拷贝字符串
FORCEINLINE void TKStrCopy(TCHAR * pDest, unsigned int uiCount, const TCHAR * pSource)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_tcscpy_s(pDest, uiCount, pSource);
#else
	return;
#endif
}

//比较字符串
FORCEINLINE int TKStrCmp(const TCHAR *String1, const TCHAR *String2)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return _tcscmp(String1, String2);
#else
	return;
#endif
}

//C++中的标准库函数，用于将多字节编码字符串转换为宽字符编码字符串，即将char*转换成wchar_t*
FORCEINLINE void TKMbsToWcs(wchar_t * Dest, unsigned int uiSizeInWord, const char * Source, unsigned int uiSizeInByte)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	mbstowcs_s(0, Dest, uiSizeInWord, Source, uiSizeInByte);
#else
	return;
#endif
}

//C++中的标准库函数，用于将宽字符编码字符串转换为多字节编码字符串，即将wchar_t*转换成char*
FORCEINLINE void TKWcsToMbs(char * Dest, unsigned int uiSizeInByte, const wchar_t * Source, unsigned int uiSizeInWord)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	wcstombs_s(0, Dest, uiSizeInByte, Source, uiSizeInWord);
#else
	return;
#endif
}

FORCEINLINE void TKOutPutDebugString(const TCHAR * pcString, ...)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	char *pArgs;
	pArgs = (char*)&pcString + sizeof(pcString);
	_vstprintf_s(TKSystem::ms_sLogBuffer, LOG_BUFFER_SIZE, pcString, pArgs);
	OutputDebugString(TKSystem::ms_sLogBuffer);
#else
	return;
#endif
}

FORCEINLINE void TKSprintf(TCHAR * _DstBuf, unsigned int _SizeInBytes, const TCHAR * _Format, ...)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	char *pArgs;
	pArgs = (char*)&_Format + sizeof(_Format);
	_vstprintf_s(_DstBuf, _SizeInBytes, _Format, pArgs);
#else
	return;
#endif
}

FORCEINLINE void TKSprintf(TCHAR * _DstBuf, unsigned int _SizeInBytes, const TCHAR * _Format, va_list pArgs)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_vstprintf_s(_DstBuf, _SizeInBytes, _Format, pArgs);
#else
	return;
#endif
}

FORCEINLINE void TKScanf(TCHAR * Buf, const TCHAR * _Format, va_list pArgs)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_stscanf_s(Buf, _Format, pArgs);
#else
	return;
#endif
}


// =============================系统信息相关函数=======================================
FORCEINLINE unsigned int TKGetCpuNum()
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	SYSTEM_INFO SyetemInfo;
	GetSystemInfo(&SyetemInfo);
	return SyetemInfo.dwNumberOfProcessors;
#else
	return 1;
#endif
}

FORCEINLINE bool TKIsMainThread()
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return TKSystem::ms_dwMainThreadID == GetCurrentThreadId();
#else
	return false;
#endif
}

FORCEINLINE void TKInitSystem()
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	TKSystem::ms_dwMainThreadID = GetCurrentThreadId();
#else
	return;
#endif
}

TK_NAMESPACE_END

#endif // !TK_SYSTEM_H