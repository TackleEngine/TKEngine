#ifndef TKLOG_H
#define TKLOG_H
#include "TKSystem.h"
#include "TKFile.h"

TK_NAMESPACE_BEGIN

    class TKSYSTEM_API TKLog : public TKFile
    {
    public:
		TKLog();
        ~TKLog();
        bool Open(const TCHAR* pFileName);
        bool WriteInfo(const TCHAR* pcString, ...) const;
    };

TK_NAMESPACE_END
#endif
