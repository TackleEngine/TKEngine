#include "TKMemManager.h"
#include "TKSystem.h"
#include <DbgHelp.h>

TK_NAMESPACE_BEGIN
TKCriticalSection TKDebugMem::ms_MemLock;
TKCriticalSection TKMemWin32::ms_MemLock;

TKMemManager::TKMemManager()
{
}

TKMemManager::~TKMemManager()
{
}

TKDebugMem::TKDebugMem()
{
    m_uiNumNewCalls = 0;
    m_uiNumDeleteCalls = 0;
    m_uiNumBlocks = 0;
    m_uiNumBytes = 0;
    m_uiMaxNumBlocks = 0;
    m_uiMaxNumBytes = 0;

    m_pHead = nullptr;
    m_pTail = nullptr;

    for (unsigned int i = 0; i < RECORD_NUM; i++)
    {
        m_uiSizeRecord[i] = 0;
    }
}

TKDebugMem::~TKDebugMem()
{
    InitDbgHelpLib();
    PrintInfo();
    FreeDbgHelpLib();
    FreeLeakMem();
}

void * TKDebugMem::Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray)
{
    ms_MemLock.Lock();
    if (!uiSize)
    {
        ms_MemLock.Unlock();
        return nullptr;
    }

    m_uiNumNewCalls++;		//增加调用new的次数
    unsigned int uiAllSize = sizeof(Block) + sizeof(unsigned int) + uiSize + sizeof(unsigned int);		//计算申请的总空间， sizeof(unsigned int)即标志位掩码
    char* pcAddr = (char*)malloc(uiAllSize);
    if (!pcAddr)
        return nullptr;

    //记录Block信息
    Block* pBlock = (Block*)pcAddr;
    pBlock->m_uiSize = uiSize;
    pBlock->m_bAlignment = (uiAlignment > 0) ? true : false;
    pBlock->m_bIsArray = bIsArray;

    pBlock->m_uiStackInfoNum = 0;
    DWORD _ebp, _esp;
    __asm mov _ebp, ebp;
    __asm mov _esp, esp;
    for (unsigned int index = 0; index < CALLSTACK_NUM; index++)
    {
        void * pAddr = (void*)ULongToPtr(*(((DWORD*)ULongToPtr(_ebp)) + 1));
        if (!pAddr)
        {
            break;
        }
        pBlock->pAddr[index] = pAddr;	//记录申请内存调用堆栈信息，然后根据dbgHelp.dll中的函数可获取地址所在源码行
        pBlock->m_uiStackInfoNum++;
        _ebp = *(DWORD*)ULongToPtr(_ebp);
        if (_ebp == 0 || 0 != (_ebp & 0xFC000000) || _ebp < _esp)
            break;
    }

    //插入节点
    InsertBlock(pBlock);
    //填写头标志位
    pcAddr += sizeof(Block);
    unsigned int * pBeginMask = (unsigned int *)(pcAddr);
    *pBeginMask = BEGIN_MASK;
    //填写尾标志位
    pcAddr += sizeof(unsigned int);
    unsigned int * pEndMask = (unsigned int *)(pcAddr + uiSize);
    *pEndMask = END_MASK;

    m_uiNumBlocks++;			//增加当前共有多少个内存块
    m_uiNumBytes += uiSize;		//增加当前共有多少个字节

    //记录当前最多申请多少内存块和字节
    if (m_uiNumBlocks > m_uiMaxNumBlocks)
    {
        m_uiMaxNumBlocks = m_uiNumBlocks;
    }
    if (m_uiNumBytes > m_uiMaxNumBytes)
    {
        m_uiMaxNumBytes = m_uiNumBytes;
    }

    //记录2到n次方范围内的内存申请次数
    unsigned int uiTwoPowerI = 1;
    int i = 0;
    for (i = 0; i <= RECORD_NUM - 2; i++, uiTwoPowerI <<= 1)
    {
        if (uiSize < uiTwoPowerI)
        {
            //记录了当前分配内存大小坐落在那个区间
            m_uiSizeRecord[i]++;
            break;
        }
    }

    if (i == RECORD_NUM - 1)
    {
        m_uiSizeRecord[i]++;
    }

    ms_MemLock.Unlock();
    return (void*)pcAddr;		//pcAddr指针指向实际分配的内存地址开头，即C部分头位置
}

void TKDebugMem::Deallocate(char * pcAddr, unsigned int uiAlignment, bool bIsArray)
{
    ms_MemLock.Lock();
    if (!pcAddr)
    {
        ms_MemLock.Unlock();
        return;
    }

    m_uiNumDeleteCalls++;
    pcAddr -= sizeof(unsigned int);

    unsigned int * pBeginMask = (unsigned int *)(pcAddr);
    TKMAC_ASSERT(*pBeginMask == BEGIN_MASK);

    //移除链表节点
    pcAddr -= sizeof(Block);
    Block* pBlock = (Block*)pcAddr;
    RemoveBlock(pBlock);

    bool bAlignment = (uiAlignment > 0) ? true : false;
    TKMAC_ASSERT(pBlock->m_bAlignment == bAlignment);
    TKMAC_ASSERT(pBlock->m_bIsArray == bIsArray);
    TKMAC_ASSERT(m_uiNumBlocks > 0 && m_uiNumBytes >= pBlock->m_uiSize);
    unsigned int * pEndMask = (unsigned int *)(pcAddr + sizeof(Block) + sizeof(unsigned int) + pBlock->m_uiSize);
    TKMAC_ASSERT(*pEndMask == END_MASK);

    m_uiNumBlocks--;
    m_uiNumBytes -= pBlock->m_uiSize;

    free(pcAddr);
    ms_MemLock.Unlock();
}

void TKDebugMem::InsertBlock(Block * pBlock)
{
    //已经有节点了
    if (m_pTail)
    {
        pBlock->m_pPrev = m_pTail;
        pBlock->m_pNext = nullptr;
        m_pTail->m_pNext = pBlock;
        m_pTail = pBlock;
    }
    else
    {
        pBlock->m_pPrev = nullptr;
        pBlock->m_pNext = nullptr;
        m_pHead = pBlock;
        m_pTail = pBlock;
    }
}

void TKDebugMem::RemoveBlock(Block * pBlock)
{
    if (pBlock->m_pPrev)
    {
        pBlock->m_pPrev->m_pNext = pBlock->m_pNext;
    }
    else
    {
        //删除的是第一个链表节点
        m_pHead = pBlock->m_pNext;
    }

    if (pBlock->m_pNext)
    {
        pBlock->m_pNext->m_pPrev = pBlock->m_pPrev;
    }
    else
    {
        m_pTail = pBlock->m_pPrev;
    }
}

void TKDebugMem::FreeLeakMem()
{
    Block* pBlock = m_pHead;
    while (pBlock)
    {
        Block* pTemp = pBlock;
        pBlock = pTemp->m_pNext;
        free((void*)pTemp);
    }
}

//========================window Debug dbghelp.dll Begin=========================================
typedef BOOL
(WINAPI
*tFMiniDumpWriteDump)
(
IN HANDLE hProcess,
IN DWORD ProcessId,
IN HANDLE hFile,
IN MINIDUMP_TYPE DumpType,
IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
);

typedef BOOL
(WINAPI
*tFSymInitialize)
(
IN HANDLE   hProcess,
IN PSTR     UserSearchPath,
IN BOOL     fInvadeProcess
);

typedef BOOL
(WINAPI
*tFStackWalk64)
(
DWORD                             MachineType,
        HANDLE                            hProcess,
HANDLE                            hThread,
        LPSTACKFRAME64                    StackFrame,
PVOID                             ContextRecord,
        PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
        PGET_MODULE_BASE_ROUTINE64        GetModuleBaseRoutine,
PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
);

typedef BOOL
(WINAPI
*tFSymFromAddr)
(
IN  HANDLE              hProcess,
IN  DWORD64             Address,
OUT PDWORD64            Displacement,
IN OUT PSYMBOL_INFO     Symbol
);

typedef BOOL
(WINAPI
*tFSymGetLineFromAddr64)
(
IN  HANDLE                  hProcess,
IN  DWORD64                 qwAddr,
OUT PDWORD                  pdwDisplacement,
OUT PIMAGEHLP_LINE64        Line64
);

typedef DWORD
(WINAPI
*tFSymGetOptions)
(
VOID
);

typedef DWORD
(WINAPI
*tFSymSetOptions)
(
IN DWORD  SymOptions
);

typedef PVOID
(WINAPI
*tFSymFunctionTableAccess64)
(
HANDLE  hProcess,
        DWORD64 AddrBase
);

typedef DWORD64
(WINAPI
*tFSymGetModuleBase64)
(
IN  HANDLE  hProcess,
IN  DWORD64  qwAddr
);

static tFMiniDumpWriteDump fnMiniDumpWriteDump = nullptr;
static tFSymInitialize fnSymInitialize = nullptr;
static tFStackWalk64 fnStackWalk64 = nullptr;
static tFSymFromAddr fnSymFromAddr = nullptr;
static tFSymGetLineFromAddr64 fnSymGetLineFromAddr64 = nullptr;
static tFSymGetOptions fnSymGetOptions = nullptr;
static tFSymSetOptions fnSymSetOptions = nullptr;
static tFSymFunctionTableAccess64 fnSymFunctionTableAccess64 = nullptr;
static tFSymGetModuleBase64 fnSymGetModuleBase64 = nullptr;

static HMODULE s_DbgHelpLib = nullptr;
static HANDLE s_Process = nullptr;

bool TKDebugMem::InitDbgHelpLib()
{
    TCHAR szDbgName[MAX_PATH];
    GetModuleFileName(nullptr, szDbgName, MAX_PATH);
    TCHAR* p = (TCHAR*)TKCsrchr(szDbgName, _T('\\'));
    if (p)
        *p = 0;
    TKStrcat(szDbgName, MAX_PATH, _T("\\dbghelp.dll"));

    //查找当前目录的DLL
    s_DbgHelpLib = LoadLibrary(szDbgName);
    if (s_DbgHelpLib == nullptr)
    {
        s_DbgHelpLib = LoadLibrary(_T("dbghelp.dll"));
        if (s_DbgHelpLib == nullptr)
            return false;
    }

    //获取dbghelp.dll中部分函数
    fnMiniDumpWriteDump = (tFMiniDumpWriteDump)GetProcAddress(s_DbgHelpLib, "MiniDumpWriteDump");
    fnSymInitialize = (tFSymInitialize)GetProcAddress(s_DbgHelpLib, "SymInitialize");
    fnStackWalk64 = (tFStackWalk64)GetProcAddress(s_DbgHelpLib, "StackWalk64");
    fnSymFromAddr = (tFSymFromAddr)GetProcAddress(s_DbgHelpLib, "SymFromAddr");
    fnSymGetLineFromAddr64 = (tFSymGetLineFromAddr64)GetProcAddress(s_DbgHelpLib, "SymGetLineFromAddr64");
    fnSymGetOptions = (tFSymGetOptions)GetProcAddress(s_DbgHelpLib, "SymGetOptions");
    fnSymSetOptions = (tFSymSetOptions)GetProcAddress(s_DbgHelpLib, "SymSetOptions");
    fnSymFunctionTableAccess64 = (tFSymFunctionTableAccess64)GetProcAddress(s_DbgHelpLib, "SymFunctionTableAccess64");
    fnSymGetModuleBase64 = (tFSymGetModuleBase64)GetProcAddress(s_DbgHelpLib, "SymGetModuleBase64");

    if (fnMiniDumpWriteDump &&
        fnSymInitialize &&
        fnStackWalk64 &&
        fnSymFromAddr &&
        fnSymGetLineFromAddr64 &&
        fnSymGetOptions &&
        fnSymSetOptions &&
        fnSymFunctionTableAccess64 &&
        fnSymGetModuleBase64)
    {
        DWORD ProcessID = GetCurrentProcessId();
        //打开一个已存在的进程对象，并返回进程句柄
        s_Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
        fnSymInitialize(s_Process, _T((PSTR)"."), TRUE);
        return true;
    }
    return false;
}

void TKDebugMem::FreeDbgHelpLib()
{
    if (s_DbgHelpLib == nullptr)
    {
        FreeLibrary(s_DbgHelpLib);
        CloseHandle(s_Process);
        s_DbgHelpLib = nullptr;
        s_Process = nullptr;
    }

    fnMiniDumpWriteDump = nullptr;
    fnSymInitialize = nullptr;
    fnStackWalk64 = nullptr;
    fnSymFromAddr = nullptr;
    fnSymGetLineFromAddr64 = nullptr;
    fnSymGetOptions = nullptr;
    fnSymSetOptions = nullptr;
    fnSymFunctionTableAccess64 = nullptr;
    fnSymGetModuleBase64 = nullptr;
}

bool TKDebugMem::GetFileAndLine(const void *pAddress, TCHAR szFile[MAX_PATH], int &line)
{
    IMAGEHLP_LINE64 Line;
    Line.SizeOfStruct = sizeof(Line);
    TKMemset(&Line, 0, sizeof(Line));
    DWORD Offset = 0;

    fnSymSetOptions(SYMOPT_LOAD_LINES);

    //找到指定地址的源代码行
    if (fnSymGetLineFromAddr64(s_Process, (DWORD64)pAddress, &Offset, &Line))
    {
#ifdef  _UNICODE
        TKMbsToWcs(szFile, MAX_PATH, Line.FileName, MAX_PATH);
#else
        TKStrCopy(szFile, MAX_PATH, Line.FileName);
#endif
        line = Line.LineNumber;
        return true;
    }
    else
    {
        DWORD error = GetLastError();
        TKOutPutDebugString(_T("SymGetLineFromAddr64 returned error: %d\n", error));
        return false;
    }
}

void TKDebugMem::PrintInfo()
{
    TKOutPutDebugString(_T("#########################  begin print leak mem  ######################\n"));
    TKOutPutDebugString(_T("New Calls Count : %d\n"), m_uiNumNewCalls);
    TKOutPutDebugString(_T("Delete Calls Count : %d\n"), m_uiNumDeleteCalls);
    TKOutPutDebugString(_T("Max Byte Count : %d\n"), m_uiMaxNumBytes);
    TKOutPutDebugString(_T("Max Block Count : %d\n"), m_uiMaxNumBlocks);
    TKOutPutDebugString(_T("Current Total Bytes Count : %d\n"), m_uiNumBytes);
    TKOutPutDebugString(_T("Current Total Blocks Count : %d\n"), m_uiNumBlocks);

    if (m_pHead)
    {
        TKOutPutDebugString(_T("Memory Leak:\n"));
    }
    else
    {
        TKOutPutDebugString(_T("No Memory Leak\n"));
    }

    Block* pBlock = m_pHead;
    static unsigned int m_uiLeakNum = 0;
    while (pBlock)
    {
        m_uiLeakNum++;
        TKOutPutDebugString(_T("$$$$$$$$$$$$$$$$  Leak %d  $$$$$$$$$$$$$$$$$\n"), m_uiLeakNum);
        TKOutPutDebugString(_T("Size: %d\n"), pBlock->m_uiSize);
        TKOutPutDebugString(_T("Is Alignment: %d\n"), pBlock->m_bAlignment);
        TKOutPutDebugString(_T("Is Array:%d\n"), pBlock->m_bIsArray);

        TCHAR szFile[MAX_PATH];
        int	line;
        for (unsigned int i = 0; i < pBlock->m_uiStackInfoNum; i++)
        {
            if (!GetFileAndLine(pBlock->pAddr[i], szFile, line))
            {
                break;
            }
            TKOutPutDebugString(_T("%s(%d)\n"), szFile, line);
        }

        TKOutPutDebugString(_T("$$$$$$$$$$$$$$$$$ Leak %d  $$$$$$$$$$$$$$$$$$$\n"), m_uiLeakNum);
        pBlock = pBlock->m_pNext;
    }

    TKOutPutDebugString(_T("leak block total num : %d\n"), m_uiLeakNum);
	TKOutPutDebugString(_T("#########################  end print leak mem  ######################\n"));
}

//========================window Debug dbghelp.dll End=========================================

//========================WIN32 Manager Begin==================================================
TKMemWin32::TKMemWin32()
{
    PageSize = 0;
    //获取WIN32系统信息
    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);
    PageSize = SysInfo.dwPageSize;
    TKMAC_ASSERT(!(PageSize&(PageSize - 1)));

    //这个是留给大于maxsize使用的
    OsTable.FirstPool = nullptr;
    OsTable.ExhaustedPool = nullptr;
    OsTable.BlockSize = 0;

    //初始化43个PoolTable
    PoolTable[0].FirstPool = nullptr;
    PoolTable[0].ExhaustedPool = nullptr;
    PoolTable[0].BlockSize = 8;
    for (DWORD i = 1; i < 5; i++)
    {
        PoolTable[i].FirstPool = nullptr;
        PoolTable[i].ExhaustedPool = nullptr;
        PoolTable[i].BlockSize = (8 << ((i + 1) >> 2)) + (2 << i);
    }
    for (DWORD i = 5; i < POOL_COUNT; i++)
    {
        PoolTable[i].FirstPool = NULL;
        PoolTable[i].ExhaustedPool = NULL;
        PoolTable[i].BlockSize = (4 + ((i + 7) & 3)) << (1 + ((i + 7) >> 2));
    }

    //建立从0～32768字节映射到PoolTable的表
    for (DWORD i = 0; i < POOL_MAX; i++)
    {
        DWORD Index;
        for (Index = 0; PoolTable[Index].BlockSize < i; Index++);
        TKMAC_ASSERT(Index < POOL_COUNT);
        MemSizeToPoolTable[i] = &PoolTable[Index];
    }

    //清空32个一级索引
    for (DWORD i = 0; i < 32; i++)
    {
        PoolIndirect[i] = NULL;
    }
    TKMAC_ASSERT(POOL_MAX - 1 == PoolTable[POOL_COUNT - 1].BlockSize);
}

TKMemWin32::~TKMemWin32()
{
    for (unsigned int i = 0; i < 32; i++)
    {
        for (unsigned int j = 0; j < 2048; j++)
        {
            if (PoolIndirect[i])
            {
                if (PoolIndirect[i][j].Mem)
                {
                    VirtualFree(PoolIndirect[i][j].Mem, 0, MEM_RELEASE);
                    PoolIndirect[i][j].Mem = nullptr;
                }

                VirtualFree(PoolIndirect[i], 0, MEM_RELEASE);
                PoolIndirect[i] = nullptr;
            }
        }
    }
}

void * TKMemWin32::Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray)
{
    ms_MemLock.Lock();
    FFreeMem* Free;
    if (uiSize < POOL_MAX)
    {
        // Allocate from pool.
        FPoolTable* Table = MemSizeToPoolTable[uiSize];
        TKMAC_ASSERT(uiSize <= Table->BlockSize);
        FPoolInfo* Pool = Table->FirstPool;
        if (!Pool)
        {
            // Must create a new pool.
            DWORD Blocks = 65536 / Table->BlockSize;
            DWORD Bytes = Blocks * Table->BlockSize;
            TKMAC_ASSERT(Blocks >= 1);
            TKMAC_ASSERT(Blocks*Table->BlockSize <= Bytes);

            // Allocate memory.
            Free = (FFreeMem*)VirtualAlloc(nullptr, Bytes, MEM_COMMIT, PAGE_READWRITE);
            if (!Free)
            {
                return nullptr;
            }

            // Create pool in the indirect table.
            FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free >> 27)];
            if (!Indirect)
            {
                Indirect = CreateIndirect();
            }
            Pool = &Indirect[((DWORD)Free >> 16) & 2047];

            // Init pool.
            Pool->Link(Table->FirstPool);
            Pool->Mem = (BYTE*)Free;
            Pool->Bytes = Bytes;
            Pool->OsBytes = Align(Bytes, PageSize);
            Pool->Table = Table;
            Pool->Taken = 0;
            Pool->FirstMem = Free;

            // Create first free item.
            Free->Blocks = Blocks;
            Free->Next = nullptr;
        }

        // Pick first available block and unlink it.
        Pool->Taken++;
        TKMAC_ASSERT(Pool->FirstMem);
        TKMAC_ASSERT(Pool->FirstMem->Blocks > 0);
        Free = (FFreeMem*)((BYTE*)Pool->FirstMem + --Pool->FirstMem->Blocks * Table->BlockSize);
        if (Pool->FirstMem->Blocks == 0)
        {
            Pool->FirstMem = Pool->FirstMem->Next;
            if (!Pool->FirstMem)
            {
                // Move to exhausted list.
                Pool->Unlink();
                Pool->Link(Table->ExhaustedPool);
            }
        }
    }
    else
    {
        // Use OS for large allocations.
        INT AlignedSize = Align(uiSize, PageSize);
        Free = (FFreeMem*)VirtualAlloc(nullptr, AlignedSize, MEM_COMMIT, PAGE_READWRITE);
        if (!Free)
        {
            return nullptr;
        }
        TKMAC_ASSERT(!((SIZE_T)Free & 65535));

        // Create indirect.
        FPoolInfo*& Indirect = PoolIndirect[((DWORD)Free >> 27)];
        if (!Indirect)
        {
            Indirect = CreateIndirect();
        }

        // Init pool.
        FPoolInfo* Pool = &Indirect[((DWORD)Free >> 16) & 2047];
        Pool->Mem = (BYTE*)Free;
        Pool->Bytes = uiSize;
        Pool->OsBytes = AlignedSize;
        Pool->Table = &OsTable;

    }
    ms_MemLock.Unlock();
    return Free;
}

void TKMemWin32::Deallocate(char * pcAddr, unsigned int uiAlignment, bool bIsArray)
{
    ms_MemLock.Lock();
    if (!pcAddr)
    {
        return;
    }

    // Windows version.
    FPoolInfo* Pool = &PoolIndirect[(DWORD)pcAddr >> 27][((DWORD)pcAddr >> 16) & 2047];
    TKMAC_ASSERT(Pool->Bytes != 0);
    if (Pool->Table != &OsTable)
    {
        // If this pool was exhausted, move to available list.
        if (!Pool->FirstMem)
        {
            Pool->Unlink();
            Pool->Link(Pool->Table->FirstPool);
        }

        // Free a pooled allocation.
        FFreeMem* Free = (FFreeMem *)pcAddr;
        Free->Blocks = 1;
        Free->Next = Pool->FirstMem;
        Pool->FirstMem = Free;


        // Free this pool.
        TKMAC_ASSERT(Pool->Taken >= 1);
        if (--Pool->Taken == 0)
        {
            // Free the OS memory.
            Pool->Unlink();
            VirtualFree(Pool->Mem, 0, MEM_RELEASE);
            Pool->Mem = nullptr;
        }
    }
    else
    {
        // Free an OS allocation.
        VirtualFree(pcAddr, 0, MEM_RELEASE);
        Pool->Mem = nullptr;
    }
    ms_MemLock.Unlock();
}

TKMemWin32::FPoolInfo* TKMemWin32::FFreeMem::GetPool()
{
    return (FPoolInfo*)((INT)this & 0xffff0000);
}

//========================WIN32 Manager End====================================================

//========================STACK Manager Begin==================================================
TKStackMem::TKStackMem(unsigned int uiDefaultChunkSize)
{
    Top = nullptr;
    End = nullptr;
    DefaultChunkSize = uiDefaultChunkSize;
    TopChunk = nullptr;
    UnusedChunks = nullptr;
    NumMarks = 0;
}

TKStackMem::~TKStackMem()
{
    FreeChunks(nullptr);
    while (UnusedChunks)
    {
        void* Old = UnusedChunks;
        UnusedChunks = UnusedChunks->Next;
        TKMemObject::GetMemManager().Deallocate((char *)Old, 0, true);
    }
}

void * TKStackMem::Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray)
{
    // Debug checks.
    TKMAC_ASSERT(uiSize >= 0);
    if (uiAlignment > 0)
    {
        TKMAC_ASSERT((uiAlignment&(uiAlignment - 1)) == 0);
    }
    TKMAC_ASSERT(Top <= End);
    TKMAC_ASSERT(NumMarks > 0);

    // Try to get memory from the current chunk.
    BYTE* Result = Top;
    if (uiAlignment > 0)
    {
        Result = (BYTE *)(((unsigned int)Top + (uiAlignment - 1)) & ~(uiAlignment - 1));
    }
    Top = Result + uiSize;

    // Make sure we didn't overflow.
    if (Top > End)
    {
        // We'd pass the end of the current chunk, so allocate a new one.
        AllocateNewChunk(uiSize + uiAlignment);
        Result = Top;
        if (uiAlignment > 0)
        {
            Result = (BYTE *)(((unsigned int)Top + (uiAlignment - 1)) & ~(uiAlignment - 1));
        }
        Top = Result + uiSize;
    }
    return Result;
}

void TKStackMem::Clear()
{
    FreeChunks(nullptr);
}

BYTE * TKStackMem::AllocateNewChunk(INT MinSize)
{
    FTaggedMemory* Chunk = nullptr;
    for (FTaggedMemory** Link = &UnusedChunks; *Link; Link = &(*Link)->Next)
    {
        // Find existing chunk.
        if ((*Link)->DataSize >= MinSize)
        {
            Chunk = *Link;
            *Link = (*Link)->Next;
            break;
        }
    }
    if (!Chunk)
    {
        // Create new chunk.
        INT DataSize = Max(MinSize, (INT)DefaultChunkSize - (INT)sizeof(FTaggedMemory));
        Chunk = (FTaggedMemory*)TKMemObject::GetMemManager().Allocate(DataSize + sizeof(FTaggedMemory), 0, true);
        Chunk->DataSize = DataSize;
    }

    Chunk->Next = TopChunk;
    TopChunk = Chunk;
    Top = Chunk->Data;
    End = Top + Chunk->DataSize;
    return Top;
}

void TKStackMem::FreeChunks(FTaggedMemory * NewTopChunk)
{
    while (TopChunk != NewTopChunk)
    {
        FTaggedMemory* RemoveChunk = TopChunk;
        TopChunk = TopChunk->Next;
        RemoveChunk->Next = UnusedChunks;
        UnusedChunks = RemoveChunk;
    }
    Top = nullptr;
    End = nullptr;
    if (TopChunk)
    {
        Top = TopChunk->Data;
        End = Top + TopChunk->DataSize;
    }
}

//========================STACK Manager End====================================================

TKMemObject::TKMemObject()
{
#ifdef USE_CUSTOM_NEW
    GetMemManager();
#endif
}

TKMemObject::~TKMemObject()
{
}

TKStackMem & TKMemObject::GetStackMemManager()
{
    static TKStackMem g_StackMemManager;
    return g_StackMemManager;
}

TKMemManager & TKMemObject::GetMemManager()
{
#ifdef _DEBUG
    static TKDebugMem g_MemManager;
#else
    static TKMemWin32 g_MemManager;
#endif

    return g_MemManager;
}

TKCMem::TKCMem()
{
}

TKCMem::~TKCMem()
{
}

void * TKCMem::Allocate(unsigned int uiSize, unsigned int uiAlignment, bool bIsArray)
{
    if (!uiSize)
    {
        return nullptr;
    }
    if (uiAlignment == 0)
    {
        return malloc(uiSize);

    }
    else
    {
        return _aligned_malloc(uiSize, uiAlignment);
    }
    return nullptr;
}

void TKCMem::Deallocate(char * pcAddr, unsigned int uiAlignment, bool bIsArray)
{
    if (!pcAddr)
    {
        return;
    }

    if (uiAlignment == 0)
    {
        free(pcAddr);

    }
    else
    {
        _aligned_free(pcAddr);
    }
}

TK_NAMESPACE_END
