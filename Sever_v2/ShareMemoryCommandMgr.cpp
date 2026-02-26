#include "ShareMemoryCommandMgr.h"
#include <cstring>

ShareMemoryCommandMgr::ShareMemoryCommandMgr()
    : hMapFile(NULL)
    , pMapView(NULL)
    , pHeader(NULL)
    , pRequestBlocks(NULL)
    , pResponseBlocks(NULL)
    , sharedMemoryName(L"ShareMemoryCommand")
    , isInitialized(false)
{
}

ShareMemoryCommandMgr::~ShareMemoryCommandMgr()
{
    Cleanup();
}

bool ShareMemoryCommandMgr::CreateMapFile(const std::wstring& name, DWORD size)
{
    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        name.c_str()
    );

    if (hMapFile == NULL)
    {
        return false;
    }

    return true;
}

bool ShareMemoryCommandMgr::OpenMapFile(const std::wstring& name)
{
    hMapFile = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        name.c_str()
    );

    if (hMapFile == NULL)
    {
        return false;
    }

    return true;
}

DWORD ShareMemoryCommandMgr::CalculateTotalSize() const
{
    if (pHeader == NULL)
        return 0;
    
    return sizeof(SharedMemoryCommandHeader) + 
           (pHeader->blockCount * pHeader->blockSize * 2);
}

void ShareMemoryCommandMgr::Cleanup()
{
    if (pMapView != NULL)
    {
        UnmapViewOfFile(pMapView);
        pMapView = NULL;
    }

    if (hMapFile != NULL)
    {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }

    pHeader = NULL;
    pRequestBlocks = NULL;
    pResponseBlocks = NULL;
    isInitialized = false;
}

bool ShareMemoryCommandMgr::IsReady() const
{
    return isInitialized && pHeader != NULL && pMapView != NULL;
}

void ShareMemoryCommandMgr::InitializeLayout()
{
    if (pHeader == NULL)
        return;

    pHeader->shmSize = CalculateTotalSize();
    pHeader->blockCount = 64;
    pHeader->blockSize = sizeof(CommandBlock);
    pHeader->version = 1;
    pHeader->reqWrite = 0;
    pHeader->reqRead = 0;
    pHeader->respWrite = 0;
    pHeader->respRead = 0;
    std::memset(pHeader->reserved, 0, sizeof(pHeader->reserved));

    BYTE* pBase = static_cast<BYTE*>(pMapView);
    pRequestBlocks = reinterpret_cast<CommandBlock*>(pBase + sizeof(SharedMemoryCommandHeader));
    pResponseBlocks = reinterpret_cast<CommandBlock*>(pBase + sizeof(SharedMemoryCommandHeader) + 
                                                      (pHeader->blockCount * pHeader->blockSize));
}

void ShareMemoryCommandMgr::Init()
{
    if (isInitialized)
        return;

    DWORD totalSize = sizeof(SharedMemoryCommandHeader) + (64 * sizeof(CommandBlock) * 2);

    if (!CreateMapFile(sharedMemoryName, totalSize))
    {
        if (!OpenMapFile(sharedMemoryName))
        {
            return;
        }
    }

    pMapView = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
    if (pMapView == NULL)
    {
        Cleanup();
        return;
    }

    pHeader = static_cast<SharedMemoryCommandHeader*>(pMapView);

    if (pHeader->version == 0)
    {
        InitializeLayout();
    }
    else
    {
        BYTE* pBase = static_cast<BYTE*>(pMapView);
        pRequestBlocks = reinterpret_cast<CommandBlock*>(pBase + sizeof(SharedMemoryCommandHeader));
        pResponseBlocks = reinterpret_cast<CommandBlock*>(pBase + sizeof(SharedMemoryCommandHeader) + 
                                                          (pHeader->blockCount * pHeader->blockSize));
    }

    isInitialized = true;
}

CommandBlock ShareMemoryCommandMgr::ReadRequestNextCommand()
{
    if (!IsReady() || pHeader == NULL || pRequestBlocks == NULL)
    {
        return CommandBlock{0};
    }

    DWORD writePos = pHeader->reqWrite;
    DWORD readPos = pHeader->reqRead;

    if (writePos == readPos)
    {
        return CommandBlock{0};
    }

    CommandBlock* pBlock = &pRequestBlocks[readPos];
    CommandBlock result = *pBlock;

    pHeader->reqRead = (readPos + 1) % pHeader->blockCount;

    return result;
}

CommandBlock ShareMemoryCommandMgr::ReadResponseNextCommand()
{
    if (!IsReady() || pHeader == NULL || pResponseBlocks == NULL)
    {
        return CommandBlock{0};
    }

    DWORD writePos = pHeader->respWrite;
    DWORD readPos = pHeader->respRead;

    if (writePos == readPos)
    {
        return CommandBlock{0};
    }

    CommandBlock* pBlock = &pResponseBlocks[readPos];
    CommandBlock result = *pBlock;

    pHeader->respRead = (readPos + 1) % pHeader->blockCount;

    return result;
}

bool ShareMemoryCommandMgr::WriteRequestNextCommand(CommandBlock command)
{
    if (!IsReady() || pHeader == NULL || pRequestBlocks == NULL)
    {
        return false;
    }

    DWORD writePos = pHeader->reqWrite;
    DWORD nextPos = (writePos + 1) % pHeader->blockCount;

    if (nextPos == pHeader->reqRead)
    {
        return false;
    }

    pRequestBlocks[writePos] = command;

    pHeader->reqWrite = nextPos;

    return true;
}

bool ShareMemoryCommandMgr::WriteResponseNextCommand(CommandBlock command)
{
    if (!IsReady() || pHeader == NULL || pResponseBlocks == NULL)
    {
        return false;
    }

    DWORD writePos = pHeader->respWrite;
    DWORD nextPos = (writePos + 1) % pHeader->blockCount;

    if (nextPos == pHeader->respRead)
    {
        return false;
    }

    pResponseBlocks[writePos] = command;

    pHeader->respWrite = nextPos;

    return true;
}
