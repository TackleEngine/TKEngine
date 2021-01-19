#include "TKLog.h"
USING_TK_NAMESPACE

TKLog::TKLog()
{
}

TKLog::~TKLog()
{
}

bool TKLog::Open(const TCHAR * pFileName)
{
    return TKFile::Open(pFileName, OM_WT);
}

bool TKLog::WriteInfo(const TCHAR * pcString, ...) const
{
    char *pArgs;
    pArgs = (char*)&pcString + sizeof(pcString);
    _vstprintf_s(TKSystem::ms_sLogBuffer, LOG_BUFFER_SIZE, pcString, pArgs);
    _ftprintf(m_pFileHandle, TKSystem::ms_sLogBuffer);
    return 1;
}
