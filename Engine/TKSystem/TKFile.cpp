#include "TKFile.h"
USING_TK_NAMESPACE

TCHAR TKFile::ms_cOpenMode[OM_MAX][5] =
{
	_T("rb"),
	_T("wb"),
	_T("rt"),
	_T("wt"),
};
unsigned int TKFile::m_uiSeekFlag[] =
{
	SEEK_CUR,
	SEEK_END,
	SEEK_SET
};

TKFile::TKFile()
{
	printf("Enter TKFile constructor ==============\n");
	m_pFileHandle = NULL;
	m_uiOpenMode = OM_MAX;
	m_uiFileSize = 0;
}

TKFile::~TKFile()
{
	printf("Enter TKFile disconstructor ==============\n");
	if (m_pFileHandle)
	{
		fclose(m_pFileHandle);
		m_pFileHandle = NULL;
	}
}

bool TKFile::Seek(unsigned int uiOffSet, unsigned int uiOrigin)
{
	if (!m_pFileHandle)
	{
		return false;
	}
	fseek(m_pFileHandle, uiOffSet, uiOrigin);
	return false;
}

bool TKFile::Open(const TCHAR * pFileName, unsigned int uiOpenMode)
{
	if (!m_pFileHandle)
	{
		return false;
	}
	if (!pFileName || m_uiOpenMode >= OM_MAX)
	{
		return false;
	}

	unsigned int uiLen = TKStrLen(pFileName);
	if (uiLen < TKMAX_PATH - 1)
	{
		if (!TKMemcpy(m_tcFileName, pFileName, uiLen + 1))
			return false;
	}
	else
	{
		return false;
	}

	m_uiOpenMode = uiOpenMode;
	if (m_uiOpenMode == OM_RB || m_uiOpenMode == OM_RT)
	{
		struct _stat64i32 kStat;
		if (_tstat(pFileName, &kStat) != 0)
		{
			return false;
		}
		m_uiFileSize = kStat.st_size;
	}
	errno_t uiError = fopen_s(&m_pFileHandle, pFileName, ms_cOpenMode[m_uiOpenMode]);
	if (uiError)
	{
		return 0;
	}
	if (!m_pFileHandle)
	{
		return 0;
	}

	return true;
}

bool TKFile::Write(const void* pBuffer, unsigned int uiSize, unsigned int uiCount)
{
	if (!m_pFileHandle)
	{
		return false;
	}

	if (!pBuffer || !uiSize || !uiCount)
	{
		return false;
	}
	fwrite(pBuffer, uiSize, uiCount, m_pFileHandle);
	return true;
}

bool TKFile::Read(void* pBuffer, unsigned int uiSize, unsigned int uiCount)
{
	if (!m_pFileHandle)
	{
		return false;
	}
	if (!pBuffer || !uiSize || !uiCount)
	{
		return false;
	}
	fread(pBuffer, uiSize, uiCount, m_pFileHandle);
	return true;
}

bool TKFile::Flush()
{
	return (fflush(m_pFileHandle) == 0);
}

bool TKFile::GetLine(void * pBuffer, unsigned int uiSize)
{
	if (!m_pFileHandle)
	{
		return false;
	}
	if (!pBuffer || !uiSize)
	{
		return false;
	}
	if (!_fgetts((TCHAR *)pBuffer, uiSize, m_pFileHandle))
		return false;
	return true;
}


