#ifndef TKFILE_H
#define TKFILE_H
#include <cstdio>
#include "TKSystem.h"

TK_NAMESPACE_BEGIN

class TKSYSTEM_API TKFile
{
public:
	//Open Mode
	enum
	{
		OM_RB,
		OM_WB,
		OM_RT,
		OM_WT,
		OM_MAX	 // 无效操作
	};
	enum
	{
		TKMAX_PATH = 256
	};
	//Seek Flag
	enum
	{
		SF_CUR,
		SF_END,
		SF_SET,
		SF_MAX	 // 无效操作
	};

	TKFile();
	~TKFile();

	//重新定位流位置 fseek
	bool Seek(unsigned int uiOffSet, unsigned int uiOrigin);
	bool Open(const TCHAR* pFileName, unsigned int uiOpenMode);
	bool Write(const void* pBuffer, unsigned int uiSize, unsigned int uiCount);
	bool Read(void* pBuffer, unsigned int uiSize, unsigned int uiCount);
	bool Flush();

	//这个函数返回pbuffer中，如果buffer大小大于取得的字符，则最后一个字符是回车，
	//倒数第2个是 LF    (Line Feed)
	bool GetLine(void * pBuffer, unsigned int uiSize);

	FORCEINLINE unsigned int GetFileSize()const
	{
		return m_uiFileSize;
	}
protected:
	static TCHAR ms_cOpenMode[OM_MAX][5];
	static unsigned int m_uiSeekFlag[];
	FILE* m_pFileHandle;			// 文件结构指针
	unsigned int m_uiOpenMode;		// 打开模式
	unsigned int m_uiFileSize;		// 文件大小
	TCHAR m_tcFileName[TKMAX_PATH]; // 文件名
};

TK_NAMESPACE_END

#endif // end TKFILE_H