#include "ShareMemoryCommandMgr.h"
#include <iostream>

// Default command block size (enough for CommandBlock structure)
const DWORD DEFAULT_COMMAND_BLOCK_SIZE = sizeof(CommandBlock) + 256;
const DWORD DEFAULT_COMMAND_BLOCK_COUNT = 100;

ShareMemoryCommandMgr::ShareMemoryCommandMgr()
    : hMapFile(INVALID_HANDLE_VALUE)
    , pMapView(nullptr)
    , pHeader(nullptr)
    , pRequestBlocks(nullptr)
    , pResponseBlocks(nullptr)
    , isInitialized(false) {
}

ShareMemoryCommandMgr::~ShareMemoryCommandMgr() {
    if (pMapView != nullptr || hMapFile != INVALID_HANDLE_VALUE) {
        Cleanup();
    }
}

void ShareMemoryCommandMgr::Init() {
    if (isInitialized) {
        return;
    }

    // Initialize command shared memory with default settings
    // Use Local\\ prefix (doesn't require admin rights)
    std::wstring commandMemoryName = L"Local\\SeverCommandMemory";

    DWORD totalSize = CalculateTotalSize();

    // Try to open existing shared memory first
    if (!OpenMapFile(commandMemoryName)) {
        // If failed, create new shared memory
        if (!CreateMapFile(commandMemoryName, totalSize)) {
            return;
        }
        InitializeLayout();
    }

    isInitialized = true;
}

DWORD ShareMemoryCommandMgr::CalculateTotalSize() const {
    // Total size: header + 2 * (block count * block size)
    DWORD size = SHARED_MEMORY_HEADER_SIZE + 
                 2 * DEFAULT_COMMAND_BLOCK_COUNT * DEFAULT_COMMAND_BLOCK_SIZE;
    return (size + 63) / 64 * 64; // 64-byte alignment
}

bool ShareMemoryCommandMgr::CreateMapFile(const std::wstring& name, DWORD size) {
    hMapFile = INVALID_HANDLE_VALUE;
    pMapView = nullptr;

    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size,
        name.c_str()
    );

    if (hMapFile == NULL) {
        DWORD error = GetLastError();
        std::wcout << L"CreateFileMappingW for command memory failed with error: " << error << std::endl;
        return false;
    }

    pMapView = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        size
    );

    if (pMapView == nullptr) {
        DWORD error = GetLastError();
        std::wcout << L"MapViewOfFile for command memory failed with error: " << error << std::endl;
        if (hMapFile != INVALID_HANDLE_VALUE && hMapFile != NULL) {
            CloseHandle(hMapFile);
        }
        hMapFile = INVALID_HANDLE_VALUE;
        return false;
    }

    sharedMemoryName = name;
    return true;
}

bool ShareMemoryCommandMgr::OpenMapFile(const std::wstring& name) {
    hMapFile = INVALID_HANDLE_VALUE;
    pMapView = nullptr;
    pHeader = nullptr;
    pRequestBlocks = nullptr;
    pResponseBlocks = nullptr;

    hMapFile = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        name.c_str()
    );

    if (hMapFile == NULL) {
        return false;
    }

    DWORD totalSize = CalculateTotalSize();
    pMapView = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        totalSize
    );

    if (pMapView == nullptr) {
        if (hMapFile != INVALID_HANDLE_VALUE && hMapFile != NULL) {
            CloseHandle(hMapFile);
        }
        hMapFile = INVALID_HANDLE_VALUE;
        return false;
    }

    // Validate memory layout
    pHeader = static_cast<SharedMemoryCommandHeader*>(pMapView);

    if (pHeader->shmSize == 0 || pHeader->blockCount == 0 || pHeader->blockSize == 0) {
        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
        hMapFile = INVALID_HANDLE_VALUE;
        pMapView = nullptr;
        pHeader = nullptr;
        return false;
    }

    // Calculate block pointers
    BYTE* basePtr = reinterpret_cast<BYTE*>(pMapView);
    pRequestBlocks = reinterpret_cast<CommandBlock*>(basePtr + SHARED_MEMORY_HEADER_SIZE);
    pResponseBlocks = reinterpret_cast<CommandBlock*>(basePtr + SHARED_MEMORY_HEADER_SIZE +
                                                      (pHeader->blockCount * pHeader->blockSize));

    sharedMemoryName = name;
    return true;
}

void ShareMemoryCommandMgr::InitializeLayout() {
    pHeader = static_cast<SharedMemoryCommandHeader*>(pMapView);
    pRequestBlocks = reinterpret_cast<CommandBlock*>(reinterpret_cast<BYTE*>(pMapView) + SHARED_MEMORY_HEADER_SIZE);
    pResponseBlocks = reinterpret_cast<CommandBlock*>(reinterpret_cast<BYTE*>(pRequestBlocks) + 
                                                       (DEFAULT_COMMAND_BLOCK_COUNT * DEFAULT_COMMAND_BLOCK_SIZE));

    // Initialize header
    pHeader->shmSize = CalculateTotalSize();
    pHeader->blockCount = DEFAULT_COMMAND_BLOCK_COUNT;
    pHeader->blockSize = DEFAULT_COMMAND_BLOCK_SIZE;
    pHeader->version = DEFAULT_PROTOCOL_VERSION;
    pHeader->reqWrite = 0;
    pHeader->reqRead = 0;
    pHeader->respWrite = 0;
    pHeader->respRead = 0;
    memset(pHeader->reserved, 0, sizeof(pHeader->reserved));

    // Initialize all command blocks
    for (DWORD i = 0; i < pHeader->blockCount; ++i) {
        pRequestBlocks[i].type = Invalid;
        pRequestBlocks[i].dataOffset = 0;
        pRequestBlocks[i].dataSize = 0;
        pRequestBlocks[i].flags = 0;

        pResponseBlocks[i].type = Invalid;
        pResponseBlocks[i].dataOffset = 0;
        pResponseBlocks[i].dataSize = 0;
        pResponseBlocks[i].flags = 0;
    }
}

void ShareMemoryCommandMgr::Cleanup() {
    if (pMapView) {
        UnmapViewOfFile(pMapView);
        pMapView = nullptr;
    }

    if (hMapFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hMapFile);
        hMapFile = INVALID_HANDLE_VALUE;
    }

    pHeader = nullptr;
    pRequestBlocks = nullptr;
    pResponseBlocks = nullptr;
    isInitialized = false;
}

bool ShareMemoryCommandMgr::IsReady() const {
    return isInitialized && pHeader && pRequestBlocks && pResponseBlocks;
}

CommandBlock ShareMemoryCommandMgr::ReadRequestNextCommand() {
    CommandBlock result = { Invalid, 0, 0, 0 };

    if (!IsReady()) {
        return result;
    }

    // Check if there's data to read (chase design: read cursor < write cursor)
    if (pHeader->reqRead >= pHeader->reqWrite) {
        return result; // No command available
    }

    DWORD blockCount = pHeader->blockCount;
    DWORD readIndex = pHeader->reqRead % blockCount;

    result = pRequestBlocks[readIndex];

    // Clear the block
    pRequestBlocks[readIndex].type = Invalid;
    pRequestBlocks[readIndex].dataOffset = 0;
    pRequestBlocks[readIndex].dataSize = 0;
    pRequestBlocks[readIndex].flags = 0;

    // Update read cursor
    pHeader->reqRead++;

    return result;
}

CommandBlock ShareMemoryCommandMgr::ReadResponseNextCommand() {
    CommandBlock result = { Invalid, 0, 0, 0 };

    if (!IsReady()) {
        return result;
    }

    // Check if there's data to read
    if (pHeader->respRead >= pHeader->respWrite) {
        return result; // No command available
    }

    DWORD blockCount = pHeader->blockCount;
    DWORD readIndex = pHeader->respRead % blockCount;

    result = pResponseBlocks[readIndex];

    // Clear the block
    pResponseBlocks[readIndex].type = Invalid;
    pResponseBlocks[readIndex].dataOffset = 0;
    pResponseBlocks[readIndex].dataSize = 0;
    pResponseBlocks[readIndex].flags = 0;

    // Update read cursor
    pHeader->respRead++;

    return result;
}

bool ShareMemoryCommandMgr::WriteRequestNextCommand(CommandBlock command) {
    if (!IsReady()) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    
    // Check if buffer is full
    if ((pHeader->reqWrite - pHeader->reqRead) >= blockCount) {
        return false; // Buffer full
    }

    DWORD writeIndex = pHeader->reqWrite % blockCount;

    pRequestBlocks[writeIndex] = command;

    // Update write cursor
    pHeader->reqWrite++;

    return true;
}

bool ShareMemoryCommandMgr::WriteResponseNextCommand(CommandBlock command) {
    if (!IsReady()) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    
    // Check if buffer is full
    if ((pHeader->respWrite - pHeader->respRead) >= blockCount) {
        return false; // Buffer full
    }

    DWORD writeIndex = pHeader->respWrite % blockCount;

    pResponseBlocks[writeIndex] = command;

    // Update write cursor
    pHeader->respWrite++;

    return true;
}
