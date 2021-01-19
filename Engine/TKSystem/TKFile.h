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
		OM_MAX	 // ��Ч����
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
		SF_MAX	 // ��Ч����
	};

	TKFile();
	~TKFile();

	//���¶�λ��λ�� fseek
	bool Seek(unsigned int uiOffSet, unsigned int uiOrigin);
	bool Open(const TCHAR* pFileName, unsigned int uiOpenMode);
	bool Write(const void* pBuffer, unsigned int uiSize, unsigned int uiCount);
	bool Read(void* pBuffer, unsigned int uiSize, unsigned int uiCount);
	bool Flush();

	//�����������pbuffer�У����buffer��С����ȡ�õ��ַ��������һ���ַ��ǻس���
	//������2���� LF    (Line Feed)
	bool GetLine(void * pBuffer, unsigned int uiSize);

	FORCEINLINE unsigned int GetFileSize()const
	{
		return m_uiFileSize;
	}
protected:
	static TCHAR ms_cOpenMode[OM_MAX][5];
	static unsigned int m_uiSeekFlag[];
	FILE* m_pFileHandle;			// �ļ��ṹָ��
	unsigned int m_uiOpenMode;		// ��ģʽ
	unsigned int m_uiFileSize;		// �ļ���С
	TCHAR m_tcFileName[TKMAX_PATH]; // �ļ���
};

TK_NAMESPACE_END

#endif // end TKFILE_H