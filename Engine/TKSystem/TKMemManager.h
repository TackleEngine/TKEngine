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

//内存管理基类
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
	*单链表的内存管理器,使用一个Block节点来管理对应的信息，给内存前后加上内存保护的标志位
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
		RECORD_NUM = 32,	//必须大于2
		CALLSTACK_NUM = 32
	};

	static TKCriticalSection ms_MemLock;
	class Block
	{
	public:
		Block()
		{
		}

		void* pAddr[CALLSTACK_NUM];		//申请内存时候的调用堆栈信息
		unsigned int m_uiStackInfoNum;	//堆栈层数
		unsigned int m_uiSize;			//申请内存空间大小
		bool m_bIsArray;				//是否是数组
		bool m_bAlignment;				//是否字节对齐
		Block* m_pPrev;					//前一个节点
		Block* m_pNext;					//后一个节点
	};

	unsigned int m_uiNumNewCalls;		//调用new的次数
	unsigned int m_uiNumDeleteCalls;	//调用delete的次数
	Block* m_pHead;						//链表头节点
	Block* m_pTail;						//链表尾节点
	unsigned int m_uiNumBlocks;			//当前共有多少个内存块
	unsigned int m_uiNumBytes;			//当前共有多少个字节
	unsigned int m_uiMaxNumBlocks;		//最多申请多少内存块
	unsigned int m_uiMaxNumBytes;		//最多申请多少字节
	unsigned int m_uiSizeRecord[RECORD_NUM];	//记录2到n次方范围内的内存申请次数

	void InsertBlock(Block* pBlock);
	void RemoveBlock(Block* pBlock);
	//TODO为了测试这些函数， 先把这些函数设置为公有
public:
	void FreeLeakMem();		//释放整个链表
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
	enum { POOL_MAX = 32768 + 1 };	//大于POOL_MAX视为大内存，不参与管理，可以直接创建和释放

	// Forward declares.
	struct FFreeMem;
	struct FPoolTable;

	// Memory pool info. 32 bytes.
	struct FPoolInfo
	{
		DWORD	    Bytes;		// Bytes allocated for pool.
		DWORD		OsBytes;	// 属于哪个链表管理者
		DWORD       Taken;      // 每分配一次就加1，释放一次就减1，如果释放后为0,那么就把Mem指向的内存空间还给32位Windows系统
		BYTE*       Mem;		// 指向32位Windows系统分配空间的首地址
		FPoolTable* Table;		// Index of pool.
		FFreeMem*   FirstMem;   // 指向可用的数据单元（神秘的4字节）
		FPoolInfo*	Next;		// 指向自己的后一个节点
		FPoolInfo**	PrevLink;	// 指向自己前一个节点

		//传入头指针，PoolTable的FirstPool或者ExhaustedPool
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
		FFreeMem*	Next;		// 在同一个PoolInfo中，下一个可用的单元
		DWORD		Blocks;		// 还有多少可用单元, 至少为1
		FPoolInfo*  GetPool();	// 在PoolInfo中的任意一个地址取出高17为就能定位这个PoolInfo
	};

	// 链表管理者
	struct FPoolTable
	{
		FPoolInfo* FirstPool;		// 没有分配完的poolinfo链表的头指针
		FPoolInfo* ExhaustedPool;	// 分配完的poolinfo链表头指针
		DWORD      BlockSize;		// 每次可以分配的内存大小
	};


	FPoolTable  PoolTable[POOL_COUNT], OsTable;
	FPoolInfo*	PoolIndirect[32];	 // 实现了二级索引，取高16位的前5位为一级索引，后11位为二级索引
	FPoolTable* MemSizeToPoolTable[POOL_MAX];
	INT			PageSize;		     // 页数

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

	//每帧结束或者开始的时候调用
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
	/*移除这个chunk和这个chunk之前的所有chunk*/
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
