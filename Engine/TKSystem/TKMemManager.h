#ifndef TK_MEMANAGER_H
#define TK_MEMANAGER_H
#include "TKSystem.h"
#include "TKSynchronize.h"
#include <Windows.h>
#include <new.h>
#include "TKUtils.h"
#include <math.h>
#include <stdlib.h>

#define TK_NEW new
#define TK_DELETE delete
TK_NAMESPACE_BEGIN

//�ڴ�������
class TKSYSTEM_API TKMemManager
{
public:
	TKMemManager();
	virtual ~TKMemManager() = 0;
	virtual void* Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray) = 0;
	virtual void  Deallocate(char* pcAddr, unsigned int uiAlignment, bool bIsArray) = 0;
};

class TKSYSTEM_API TKCMem : public TKMemManager
{
public:
	TKCMem();
	~TKCMem();
	void* Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray);
	void Deallocate(char* pcAddr, unsigned int uiAlignment, bool bIsArray);
};

/**
	*��������ڴ������,ʹ��һ��Block�ڵ��������Ӧ����Ϣ�����ڴ�ǰ������ڴ汣���ı�־λ
	Block	BeginMask		EndMask
	-----------------------------
	|		|	|			|	|
	|	A	| B	|	  C		| D |
	|		|	|			|	|
	-----------------------------
					Alloc Mem
*/
class TKSYSTEM_API TKDebugMem : public TKMemManager
{
public:
	TKDebugMem();
	~TKDebugMem();
	virtual void* Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray);
	virtual void  Deallocate(char* pcAddr, unsigned int uiAlignment, bool bIsArray);

private:
	enum
	{
		BEGIN_MASK = 0xDEADC0DE,
		END_MASK = 0xDEADC0DE,
		RECORD_NUM = 32,	//�������2
		CALLSTACK_NUM = 32
	};

	static TKCriticalSection ms_MemLock;
	class Block
	{
	public:
		Block()
		{
		}

		void* pAddr[CALLSTACK_NUM];		//�����ڴ�ʱ��ĵ��ö�ջ��Ϣ
		unsigned int m_uiStackInfoNum;	//��ջ����
		unsigned int m_uiSize;			//�����ڴ�ռ��С
		bool m_bIsArray;				//�Ƿ�������
		bool m_bAlignment;				//�Ƿ��ֽڶ���
		Block* m_pPrev;					//ǰһ���ڵ�
		Block* m_pNext;					//��һ���ڵ�
	};

	unsigned int m_uiNumNewCalls;		//����new�Ĵ���
	unsigned int m_uiNumDeleteCalls;	//����delete�Ĵ���
	Block* m_pHead;						//����ͷ�ڵ�
	Block* m_pTail;						//����β�ڵ�
	unsigned int m_uiNumBlocks;			//��ǰ���ж��ٸ��ڴ��
	unsigned int m_uiNumBytes;			//��ǰ���ж��ٸ��ֽ�
	unsigned int m_uiMaxNumBlocks;		//�����������ڴ��
	unsigned int m_uiMaxNumBytes;		//�����������ֽ�
	unsigned int m_uiSizeRecord[RECORD_NUM];	//��¼2��n�η���Χ�ڵ��ڴ��������

	void InsertBlock(Block* pBlock);
	void RemoveBlock(Block* pBlock);
	//TODOΪ�˲�����Щ������ �Ȱ���Щ��������Ϊ����
public:
	void FreeLeakMem();		//�ͷ���������
	bool InitDbgHelpLib();
	void FreeDbgHelpLib();
	bool GetFileAndLine(const void* pAddress, TCHAR szFile[MAX_PATH], int &line);
	void PrintInfo();
};

//this code copy from U3 FMallocWindows.h
class TKSYSTEM_API TKMemWin32 : public TKMemManager
{
public:
	TKMemWin32();
	~TKMemWin32();
	virtual void* Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray);
	virtual void  Deallocate(char* pcAddr, unsigned int uiAlignment, bool bIsArray);
private:
	static TKCriticalSection ms_MemLock;
	// Counts.
	enum { POOL_COUNT = 42 };
	enum { POOL_MAX = 32768 + 1 };	//����POOL_MAX��Ϊ���ڴ棬�������������ֱ�Ӵ������ͷ�

	// Forward declares.
	struct FFreeMem;
	struct FPoolTable;

	// Memory pool info. 32 bytes.
	struct FPoolInfo
	{
		DWORD	    Bytes;		// Bytes allocated for pool.
		DWORD		OsBytes;	// �����ĸ����������
		DWORD       Taken;      // ÿ����һ�ξͼ�1���ͷ�һ�ξͼ�1������ͷź�Ϊ0,��ô�Ͱ�Memָ����ڴ�ռ仹��32λWindowsϵͳ
		BYTE*       Mem;		// ָ��32λWindowsϵͳ����ռ���׵�ַ
		FPoolTable* Table;		// Index of pool.
		FFreeMem*   FirstMem;   // ָ����õ����ݵ�Ԫ�����ص�4�ֽڣ�
		FPoolInfo*	Next;		// ָ���Լ��ĺ�һ���ڵ�
		FPoolInfo**	PrevLink;	// ָ���Լ�ǰһ���ڵ�

		//����ͷָ�룬PoolTable��FirstPool����ExhaustedPool
		void Link(FPoolInfo*& Before)
		{
			if (Before)
			{
				Before->PrevLink = &Next;
			}
			Next = Before;
			PrevLink = &Before;
			Before = this;
		}
		void Unlink()
		{
			if (Next)
			{
				Next->PrevLink = PrevLink;
			}
			*PrevLink = Next;
		}
	};

	// Information about a piece of free memory. 8 bytes.
	struct FFreeMem
	{
		FFreeMem*	Next;		// ��ͬһ��PoolInfo�У���һ�����õĵ�Ԫ
		DWORD		Blocks;		// ���ж��ٿ��õ�Ԫ, ����Ϊ1
		FPoolInfo*  GetPool();	// ��PoolInfo�е�����һ����ַȡ����17Ϊ���ܶ�λ���PoolInfo
	};

	// ���������
	struct FPoolTable
	{
		FPoolInfo* FirstPool;		// û�з������poolinfo�����ͷָ��
		FPoolInfo* ExhaustedPool;	// �������poolinfo����ͷָ��
		DWORD      BlockSize;		// ÿ�ο��Է�����ڴ��С
	};


	FPoolTable  PoolTable[POOL_COUNT], OsTable;
	FPoolInfo*	PoolIndirect[32];	 // ʵ���˶���������ȡ��16λ��ǰ5λΪһ����������11λΪ��������
	FPoolTable* MemSizeToPoolTable[POOL_MAX];
	INT			PageSize;		     // ҳ��

	FPoolInfo* CreateIndirect()
	{
		FPoolInfo* Indirect = (FPoolInfo*)VirtualAlloc(NULL, 2048 * sizeof(FPoolInfo), MEM_COMMIT, PAGE_READWRITE);
		if (!Indirect)
		{
			return NULL;
		}
		return Indirect;
	}

};

// this is Stack Mem ,it will be clear every tick ,now No considering thread safe .
class TKSYSTEM_API TKStackMem : public TKMemManager
{
public:
	TKStackMem(unsigned int uiDefaultChunkSize = 65536);
	~TKStackMem();
	void* Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray);
	void Deallocate(char* pcAddr, unsigned int uiAlignment, bool bIsArray)
	{
		return;
	}
	template<class T>
	friend class TKStackMemAlloc;

	//ÿ֡�������߿�ʼ��ʱ�����
	void Clear();
private:

	// Types.
	struct FTaggedMemory
	{
		FTaggedMemory* Next;
		INT DataSize;
		BYTE Data[1];
	};

	// Variables.
	BYTE*			Top;				// Top of current chunk (Top<=End).
	BYTE*			End;				// End of current chunk.
	unsigned int	DefaultChunkSize;	// Maximum chunk size to allocate.
	FTaggedMemory*	TopChunk;			// Only chunks 0..ActiveChunks-1 are valid.

	/** The memory chunks that have been allocated but are currently unused. */
	FTaggedMemory*	UnusedChunks;

	/** The number of marks on this stack. */
	INT NumMarks;

	/**
	* Allocate a new chunk of memory of at least MinSize size,
	* and return it aligned to Align. Updates the memory stack's
	* Chunks table and ActiveChunks counter.
	*/
	BYTE* AllocateNewChunk(INT MinSize);

	/** Frees the chunks above the specified chunk on the stack. */
	/*�Ƴ����chunk�����chunk֮ǰ������chunk*/
	void FreeChunks(FTaggedMemory* NewTopChunk);
};

// if the class has memory alloc , you must inherit from TKMemObject , so you can use the MemManger any where
class TKSYSTEM_API TKMemObject
{
public:
	TKMemObject();
	~TKMemObject();
	static TKStackMem& GetStackMemManager();
	static TKMemManager& GetMemManager();
};
typedef TKMemManager& (*CEMemManagerFun)();

template<class T>
class TKStackMemAlloc : public TKMemObject
{
public:
	// Constructors.
	TKStackMemAlloc(unsigned int uiNum = 0, unsigned int uiAlignment = 0)
	{
		m_uiNum = uiNum;
		Top = GetStackMemManager().Top;
		SavedChunk = GetStackMemManager().TopChunk;
		// Track the number of outstanding marks on the stack.
		GetStackMemManager().NumMarks++;
		if (m_uiNum > 0)
		{
			m_pPtr = (T *)GetStackMemManager().Allocate(uiNum * sizeof(T), uiAlignment, 0);
			TKMAC_ASSERT(m_pPtr);
			if (ValueBase<T>::NeedsConstructor)
			{
				for (unsigned int i = 0; i < uiNum; i++)
				{
					TK_NEW(m_pPtr + i)T();
				}
			}
		}
	}

	/** Destructor. */
	~TKStackMemAlloc()
	{

		if (m_uiNum > 0)
		{
			if (ValueBase<T>::NeedsDestructor)
			{
				for (unsigned int i = 0; i < m_uiNum; i++)
				{
					(m_pPtr + i)->~T();
				}
			}
		}
		// Track the number of outstanding marks on the stack.
		--GetStackMemManager().NumMarks;

		// Unlock any new chunks that were allocated.
		if (SavedChunk != GetStackMemManager().TopChunk)
			GetStackMemManager().FreeChunks(SavedChunk);

		// Restore the memory stack's state.
		GetStackMemManager().Top = Top;

		// Ensure that the mark is only popped once by clearing the top pointer.
		Top = NULL;
	}

	FORCEINLINE T * GetPtr()const
	{
		return m_pPtr;
	}
	FORCEINLINE unsigned int GetNum() const
	{
		return m_uiNum;
	}
private:
	BYTE* Top;
	TKStackMem::FTaggedMemory* SavedChunk;
	T * m_pPtr;
	unsigned int m_uiNum;
};

TK_NAMESPACE_END

#define USE_CUSTOM_NEW
#ifdef USE_CUSTOM_NEW
FORCEINLINE void* operator new(size_t uiSize)
{
	return TKEngine::TKMemObject::GetMemManager().Allocate((unsigned int)uiSize, 0, false);
}
FORCEINLINE void* operator new[](size_t uiSize)
{
	return TKEngine::TKMemObject::GetMemManager().Allocate((unsigned int)uiSize, 0, true);
}

FORCEINLINE void operator delete (void* pvAddr)
{
	return TKEngine::TKMemObject::GetMemManager().Deallocate((char *)pvAddr, 0, false);
}
FORCEINLINE void operator delete[](void* pvAddr)
{
	return TKEngine::TKMemObject::GetMemManager().Deallocate((char *)pvAddr, 0, true);
}
#endif


#define TKMAC_DELETE(p) if(p){TK_DELETE p; p = 0;}
#define TKMAC_DELETEA(p) if(p){TK_DELETE []p; p = 0;}
#define TKMAC_DELETEAB(p,num) if(p){ for(int i = 0 ; i < num ; i++) TKMAC_DELETEA(p[i]); TKMAC_DELETEA(p);}
	// use by inner mac
	template<typename T>
FORCEINLINE void TKDelete(T * p)
{
	if (p) { TK_DELETE p; p = 0; }
}
template<typename T>
FORCEINLINE void TKDeleteA(T * p)
{
	if (p) { TK_DELETE[]p; p = 0; }
}
template<typename T, typename N>
FORCEINLINE void TKDeleteAB(T * p, N num)
{
	if (p) { for (int i = 0; i < num; i++) TKMAC_DELETEA(p[i]); TKMAC_DELETEA(p); }
}

#endif // !TK_MEMANAGER_H
