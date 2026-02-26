#include "SharedMemoryDateManager.h"
#include <cstring>

SharedMemoryDateManager::SharedMemoryDateManager()
    : hMapFile(NULL)
    , pMapView(NULL)
    , pHeader(NULL)
    , pRequestBlocks(NULL)
    , pResponseBlocks(NULL)
    , sharedMemoryName(L"")
    , isInitialized(false)
    , totalBlockSize(0)
{
}

SharedMemoryDateManager::~SharedMemoryDateManager()
{
    Cleanup();
}

bool SharedMemoryDateManager::CreateMapFile(DWORD size)
{
    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        sharedMemoryName.c_str()
    );

    if (hMapFile == NULL)
    {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, sharedMemoryName.c_str());
            if (hMapFile == NULL)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool SharedMemoryDateManager::OpenMapFile()
{
    hMapFile = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        sharedMemoryName.c_str()
    );

    if (hMapFile == NULL)
    {
        return false;
    }

    return true;
}

DWORD SharedMemoryDateManager::CalculateTotalSize() const
{
    return sizeof(SharedMemoryDateHeader) + (totalBlockSize * 2);
}

void SharedMemoryDateManager::Cleanup()
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

bool SharedMemoryDateManager::Initialize(const std::wstring& name, DWORD blockSize, DWORD blockCount)
{
    if (isInitialized)
        return true;

    sharedMemoryName = name;
    totalBlockSize = blockSize * blockCount;

    DWORD totalSize = CalculateTotalSize();

    if (!CreateMapFile(totalSize))
    {
        if (!OpenMapFile())
        {
            return false;
        }
    }

    pMapView = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
    if (pMapView == NULL)
    {
        Cleanup();
        return false;
    }

    pHeader = static_cast<SharedMemoryDateHeader*>(pMapView);

    if (pHeader->version == 0)
    {
        pHeader->shmSize = totalSize;
        pHeader->blockCount = blockCount;
        pHeader->blockSize = blockSize;
        pHeader->version = DEFAULT_PROTOCOL_VERSION;
        pHeader->reqWrite = 0;
        pHeader->reqRead = 0;
        pHeader->respWrite = 0;
        pHeader->respRead = 0;
        std::memset(pHeader->reserved, 0, sizeof(pHeader->reserved));
    }

    BYTE* pBase = static_cast<BYTE*>(pMapView);
    pRequestBlocks = reinterpret_cast<DateBlock*>(pBase + sizeof(SharedMemoryDateHeader));
    pResponseBlocks = reinterpret_cast<DateBlock*>(pBase + sizeof(SharedMemoryDateHeader) + totalBlockSize);

    isInitialized = true;
    return true;
}

bool SharedMemoryDateManager::IsReady() const
{
    return isInitialized && pHeader != NULL && pMapView != NULL;
}

bool SharedMemoryDateManager::WriteRequest(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType)
{
    if (!IsReady() || pHeader == NULL || pRequestBlocks == NULL)
    {
        return false;
    }

    if (data == NULL || dataSize == 0)
    {
        return false;
    }

    DWORD writePos = pHeader->reqWrite;
    DWORD nextPos = (writePos + 1) % pHeader->blockCount;

    if (nextPos == pHeader->reqRead)
    {
        return false;
    }

    DateBlock* pBlock = reinterpret_cast<DateBlock*>(
        reinterpret_cast<BYTE*>(pRequestBlocks) + (writePos * pHeader->blockSize)
    );

    pBlock->commandId = commandId;
    pBlock->dataSize = dataSize;
    pBlock->dataType = dataType;

    DWORD copySize = dataSize;
    if (copySize > pHeader->blockSize - sizeof(DateBlock) + 1)
    {
        copySize = pHeader->blockSize - sizeof(DateBlock) + 1;
    }

    std::memcpy(pBlock->payload, data, copySize);

    pHeader->reqWrite = nextPos;

    return true;
}

bool SharedMemoryDateManager::WriteResponse(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType)
{
    if (!IsReady() || pHeader == NULL || pResponseBlocks == NULL)
    {
        return false;
    }

    if (data == NULL || dataSize == 0)
    {
        return false;
    }

    DWORD writePos = pHeader->respWrite;
    DWORD nextPos = (writePos + 1) % pHeader->blockCount;

    if (nextPos == pHeader->respRead)
    {
        return false;
    }

    DateBlock* pBlock = reinterpret_cast<DateBlock*>(
        reinterpret_cast<BYTE*>(pResponseBlocks) + (writePos * pHeader->blockSize)
    );

    pBlock->commandId = commandId;
    pBlock->dataSize = dataSize;
    pBlock->dataType = dataType;

    DWORD copySize = dataSize;
    if (copySize > pHeader->blockSize - sizeof(DateBlock) + 1)
    {
        copySize = pHeader->blockSize - sizeof(DateBlock) + 1;
    }

    std::memcpy(pBlock->payload, data, copySize);

    pHeader->respWrite = nextPos;

    return true;
}

bool SharedMemoryDateManager::ReadRequest(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType)
{
    if (!IsReady() || pHeader == NULL || pRequestBlocks == NULL || buffer == NULL)
    {
        return false;
    }

    DWORD writePos = pHeader->reqWrite;
    DWORD readPos = pHeader->reqRead;

    if (writePos == readPos)
    {
        return false;
    }

    DateBlock* pBlock = reinterpret_cast<DateBlock*>(
        reinterpret_cast<BYTE*>(pRequestBlocks) + (readPos * pHeader->blockSize)
    );

    outCommandId = pBlock->commandId;
    outDataType = pBlock->dataType;

    DWORD copySize = pBlock->dataSize;
    if (copySize > bufferSize)
    {
        copySize = bufferSize;
    }
    if (copySize > pHeader->blockSize - sizeof(DateBlock) + 1)
    {
        copySize = pHeader->blockSize - sizeof(DateBlock) + 1;
    }

    std::memcpy(buffer, pBlock->payload, copySize);

    pHeader->reqRead = (readPos + 1) % pHeader->blockCount;

    return true;
}

bool SharedMemoryDateManager::ReadResponse(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType)
{
    if (!IsReady() || pHeader == NULL || pResponseBlocks == NULL || buffer == NULL)
    {
        return false;
    }

    DWORD writePos = pHeader->respWrite;
    DWORD readPos = pHeader->respRead;

    if (writePos == readPos)
    {
        return false;
    }

    DateBlock* pBlock = reinterpret_cast<DateBlock*>(
        reinterpret_cast<BYTE*>(pResponseBlocks) + (readPos * pHeader->blockSize)
    );

    outCommandId = pBlock->commandId;
    outDataType = pBlock->dataType;

    DWORD copySize = pBlock->dataSize;
    if (copySize > bufferSize)
    {
        copySize = bufferSize;
    }
    if (copySize > pHeader->blockSize - sizeof(DateBlock) + 1)
    {
        copySize = pHeader->blockSize - sizeof(DateBlock) + 1;
    }

    std::memcpy(buffer, pBlock->payload, copySize);

    pHeader->respRead = (readPos + 1) % pHeader->blockCount;

    return true;
}

DWORD SharedMemoryDateManager::GetShmSize() const
{
    if (pHeader == NULL)
        return 0;
    return pHeader->shmSize;
}

DWORD SharedMemoryDateManager::GetBlockCount() const
{
    if (pHeader == NULL)
        return 0;
    return pHeader->blockCount;
}

DWORD SharedMemoryDateManager::GetBlockSize() const
{
    if (pHeader == NULL)
        return 0;
    return pHeader->blockSize;
}

DWORD SharedMemoryDateManager::GetVersion() const
{
    if (pHeader == NULL)
        return 0;
    return pHeader->version;
}
