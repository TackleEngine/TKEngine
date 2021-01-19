/*
  * ��װ��һЩ���ú���
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

//ȥ��ģ�嵼������ľ���
// #pragma warning(disable:4595)

TK_NAMESPACE_BEGIN

/*
	����:
	_stdcall��__cdecl��__fastcall�����ֺ�������Э��,��������Э���Ӱ�캯����������ջ��ʽ��ջ�����ݵ������ʽ�������������������ι����;
	1.����Э�鳣�ó���:
		__stdcall��Windows APIĬ�ϵĺ�������Э��;
		__cdecl��C/C++Ĭ�ϵĺ�������Э��;
		__fastcall�������ڶ�����Ҫ��ϸߵĳ���;
	2.����������ջ��ʽ:
		__stdcall��������������������ջ;
		_cdecl��������������������ջ;
		__fastcall������ʼ������4�ֽڵĲ�������CPU��ECX��EDX�Ĵ����������������������ջ;
	3.ջ�����������ʽ:
		__stdcall���������ý������ɱ����ú������ջ������;
		__cdecl���������ý������ɺ������������ջ������;
		__fastcall���������ý������ɱ����ú������ջ������;
*/
#define THREAD_CALLBACK	__stdcall

#define LOG_BUFFER_SIZE 65535
class TKSYSTEM_API TKSystem
{
public:
	static TCHAR ms_sLogBuffer[LOG_BUFFER_SIZE];
	static DWORD ms_dwMainThreadID;
};

//�ڴ濽������
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

//ʵ��ԭ���ԵļӼ�
FORCEINLINE void TKLockIncrement(long* pRefCcount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_InterlockedIncrement(pRefCcount);
#else
	return;
#endif
}

//ʵ��ԭ���ԵļӼ�
FORCEINLINE void TKLockedDecrement(long * pRefCount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_InterlockedDecrement(pRefCount);
#else
	return;
#endif
}

//�ڴ��ʼ��
FORCEINLINE void TKMemset(void *pDest, int iVal, unsigned int uiCount)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	memset(pDest, iVal, uiCount);
#else
	return;
#endif
}

//��ȡUnicode�ַ�������
FORCEINLINE unsigned int TKStrLen(const TCHAR* pStr)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return (unsigned int)_tcslen(pStr);
#else
	return 0;
#endif
}

//�ַ���ƴ��
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

//�����ַ���
FORCEINLINE void TKStrCopy(TCHAR * pDest, unsigned int uiCount, const TCHAR * pSource)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	_tcscpy_s(pDest, uiCount, pSource);
#else
	return;
#endif
}

//�Ƚ��ַ���
FORCEINLINE int TKStrCmp(const TCHAR *String1, const TCHAR *String2)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	return _tcscmp(String1, String2);
#else
	return;
#endif
}

//C++�еı�׼�⺯�������ڽ����ֽڱ����ַ���ת��Ϊ���ַ������ַ���������char*ת����wchar_t*
FORCEINLINE void TKMbsToWcs(wchar_t * Dest, unsigned int uiSizeInWord, const char * Source, unsigned int uiSizeInByte)
{
#if TK_PLATFORM == TK_PLATFORM_WIN
	mbstowcs_s(0, Dest, uiSizeInWord, Source, uiSizeInByte);
#else
	return;
#endif
}

//C++�еı�׼�⺯�������ڽ����ַ������ַ���ת��Ϊ���ֽڱ����ַ���������wchar_t*ת����char*
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


// =============================ϵͳ��Ϣ��غ���=======================================
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