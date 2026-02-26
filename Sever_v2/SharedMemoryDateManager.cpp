#include "SharedMemoryDateManager.h"
#include <algorithm>

SharedMemoryDateManager::SharedMemoryDateManager() 
    : hMapFile(INVALID_HANDLE_VALUE)
    , pMapView(nullptr)
    , pHeader(nullptr)
    , pRequestBlocks(nullptr)
    , pResponseBlocks(nullptr)
    , isInitialized(false)
    , totalBlockSize(0) {
}

SharedMemoryDateManager::~SharedMemoryDateManager() {
    // 更安全的析构函数检查
    if (pMapView != nullptr || hMapFile != INVALID_HANDLE_VALUE) {
        Cleanup();
    }
}

bool SharedMemoryDateManager::Initialize(const std::wstring& name, DWORD blockSize, DWORD blockCount) {
    if (isInitialized) {
        return true;
    }

    // 验证参数
    if (blockSize < MIN_BLOCK_SIZE || blockSize > MAX_BLOCK_SIZE) {
        return false;
    }
    if (blockCount < MIN_BLOCK_COUNT || blockCount > MAX_BLOCK_COUNT) {
        return false;
    }

    sharedMemoryName = name;
    totalBlockSize = blockSize;

    DWORD totalSize = CalculateTotalSize();
    
    // 尝试打开现有的共享内存
    if (!OpenMapFile()) {
        // 如果打开失败，创建新的共享内存
        if (!CreateMapFile(totalSize)) {
            return false;
        }
        InitializeLayout();
    }

    isInitialized = true;
    return true;
}

void SharedMemoryDateManager::Cleanup() {
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

bool SharedMemoryDateManager::IsReady() const {
    return isInitialized && pHeader && pRequestBlocks && pResponseBlocks;
}

bool SharedMemoryDateManager::CreateMapFile(DWORD size) {
    // 确保初始状态干净
    hMapFile = INVALID_HANDLE_VALUE;
    pMapView = nullptr;

    hMapFile = CreateFileMappingW(
        INVALID_HANDLE_VALUE,    // 使用页面文件
        NULL,                    // 默认安全属性
        PAGE_READWRITE,          // 读写权限
        0,                       // 最大尺寸高32位
        size,                    // 最大尺寸低32位
        sharedMemoryName.c_str() // 共享内存名称
    );

    if (hMapFile == NULL) {
        DWORD error = GetLastError();
        std::wcout << L"CreateFileMappingW failed with error: " << error << std::endl;
        return false;
    }

    pMapView = MapViewOfFile(
        hMapFile,                // 文件映射对象句柄
        FILE_MAP_ALL_ACCESS,     // 读写访问
        0,                       // 文件偏移高32位
        0,                       // 文件偏移低32位
        size                     // 映射视图大小
    );

    if (pMapView == nullptr) {
        DWORD error = GetLastError();
        std::wcout << L"MapViewOfFile failed with error: " << error << std::endl;
        // 安全地关闭句柄
        if (hMapFile != INVALID_HANDLE_VALUE && hMapFile != NULL) {
            CloseHandle(hMapFile);
        }
        hMapFile = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

bool SharedMemoryDateManager::OpenMapFile() {
    // 确保初始状态干净
    hMapFile = INVALID_HANDLE_VALUE;
    pMapView = nullptr;
    pHeader = nullptr;
    pRequestBlocks = nullptr;
    pResponseBlocks = nullptr;

    hMapFile = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,     // 读写访问
        FALSE,                   // 不继承句柄
        sharedMemoryName.c_str() // 共享内存名称
    );

    if (hMapFile == NULL) {
        DWORD error = GetLastError();
        std::wcout << L"OpenFileMappingW failed with error: " << error << std::endl;
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
        DWORD error = GetLastError();
        std::wcout << L"MapViewOfFile failed with error: " << error << std::endl;
        // 安全地关闭句柄
        if (hMapFile != INVALID_HANDLE_VALUE && hMapFile != NULL) {
            CloseHandle(hMapFile);
        }
        hMapFile = INVALID_HANDLE_VALUE;
        return false;
    }

    // 验证现有内存布局
    pHeader = static_cast<SharedMemoryDateHeader*>(pMapView);
    
    // 验证头部数据的有效性
    if (pHeader->shmSize == 0 || pHeader->blockCount == 0 || pHeader->blockSize == 0) {
        std::cout << "Invalid shared memory layout detected" << std::endl;
        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
        hMapFile = INVALID_HANDLE_VALUE;
        pMapView = nullptr;
        pHeader = nullptr;
        return false;
    }
    
    // 安全计算块指针位置
    BYTE* basePtr = reinterpret_cast<BYTE*>(pMapView);
    pRequestBlocks = reinterpret_cast<DateBlock*>(basePtr + SHARED_MEMORY_HEADER_SIZE);
    pResponseBlocks = reinterpret_cast<DateBlock*>(basePtr + SHARED_MEMORY_HEADER_SIZE + 
                                              (pHeader->blockCount * pHeader->blockSize));

    // 验证版本和大小
    if (pHeader->version != DEFAULT_PROTOCOL_VERSION || 
        pHeader->shmSize != totalSize ||
        pHeader->blockSize != totalBlockSize) {
        std::cout << "Version or configuration mismatch, reinitializing" << std::endl;
        InitializeLayout();
    }

    return true;
}

void SharedMemoryDateManager::InitializeLayout() {
    pHeader = static_cast<SharedMemoryDateHeader*>(pMapView);
    pRequestBlocks = reinterpret_cast<DateBlock*>(reinterpret_cast<BYTE*>(pMapView) + SHARED_MEMORY_HEADER_SIZE);
    pResponseBlocks = reinterpret_cast<DateBlock*>(reinterpret_cast<BYTE*>(pRequestBlocks) + (pHeader->blockCount * pHeader->blockSize));

    std::cout << "Initializing kernel program" << std::endl;

    // 初始化头部信息
    pHeader->shmSize = CalculateTotalSize();
    pHeader->blockCount = (CalculateTotalSize() - SHARED_MEMORY_HEADER_SIZE) / (2 * totalBlockSize);
    pHeader->blockSize = totalBlockSize;
    pHeader->version = DEFAULT_PROTOCOL_VERSION;
    pHeader->reqWrite = 0;
    pHeader->reqRead = 0;
    pHeader->respWrite = 0;
    pHeader->respRead = 0;
    memset(pHeader->reserved, 0, sizeof(pHeader->reserved));

    // 初始化所有block为可用状态
    DWORD blockCount = pHeader->blockCount;
    for (DWORD i = 0; i < blockCount; ++i) {
        pRequestBlocks[i].commandId = 0;
        pRequestBlocks[i].dataSize = 0;
        pRequestBlocks[i].dataType = 0;
        pResponseBlocks[i].commandId = 0;
        pResponseBlocks[i].dataSize = 0;
        pResponseBlocks[i].dataType = 0;
    }
}

DWORD SharedMemoryDateManager::CalculateTotalSize() const {
    // 计算总大小：头部 + 2 * (block数量 * block大小)
    // 确保64字节对齐
    DWORD size = SHARED_MEMORY_HEADER_SIZE + 2 * ((totalBlockSize + 63) / 64 * 64) * 100; // 假设100个block
    return (size + 63) / 64 * 64; // 64字节对齐
}

bool SharedMemoryDateManager::WriteRequest(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType) {
    if (!IsReady() || data == nullptr || dataSize == 0) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    DWORD writeIndex = pHeader->reqWrite % blockCount;
    
    // 检查是否有空间
    if ((pHeader->reqWrite - pHeader->reqRead) >= blockCount) {
        return false; // 缓冲区满
    }

    DateBlock& block = pRequestBlocks[writeIndex];
    
    // 检查block大小是否足够
    if (sizeof(DateBlock) + dataSize > pHeader->blockSize) {
        return false;
    }

    // 写入数据
    block.commandId = commandId;
    block.dataSize = dataSize;
    block.dataType = dataType;
    memcpy(block.payload, data, dataSize);

    // 更新写游标
    pHeader->reqWrite++;

    return true;
}

bool SharedMemoryDateManager::WriteResponse(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType) {
    if (!IsReady() || data == nullptr || dataSize == 0) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    DWORD writeIndex = pHeader->respWrite % blockCount;
    
    // 检查是否有空间
    if ((pHeader->respWrite - pHeader->respRead) >= blockCount) {
        return false; // 缓冲区满
    }

    DateBlock& block = pResponseBlocks[writeIndex];
    
    // 检查block大小是否足够
    if (sizeof(DateBlock) + dataSize > pHeader->blockSize) {
        return false;
    }

    // 写入数据
    block.commandId = commandId;
    block.dataSize = dataSize;
    block.dataType = dataType;
    memcpy(block.payload, data, dataSize);

    // 更新写游标
    pHeader->respWrite++;

    return true;
}

bool SharedMemoryDateManager::ReadRequest(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType) {
    if (!IsReady() || buffer == nullptr || bufferSize == 0) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    
    // 检查是否有数据可读
    if (pHeader->reqRead >= pHeader->reqWrite) {
        return false;
    }

    DWORD readIndex = pHeader->reqRead % blockCount;
    DateBlock& block = pRequestBlocks[readIndex];

    // 检查缓冲区大小是否足够
    if (bufferSize < block.dataSize) {
        return false;
    }

    // 读取数据
    memcpy(buffer, block.payload, block.dataSize);
    outCommandId = block.commandId;
    outDataType = block.dataType;

    // 清空block数据
    block.commandId = 0;
    block.dataSize = 0;
    block.dataType = 0;

    // 更新读游标
    pHeader->reqRead++;

    return true;
}

bool SharedMemoryDateManager::ReadResponse(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType) {
    if (!IsReady() || buffer == nullptr || bufferSize == 0) {
        return false;
    }

    DWORD blockCount = pHeader->blockCount;
    
    // 检查是否有数据可读
    if (pHeader->respRead >= pHeader->respWrite) {
        return false;
    }

    DWORD readIndex = pHeader->respRead % blockCount;
    DateBlock& block = pResponseBlocks[readIndex];

    // 检查缓冲区大小是否足够
    if (bufferSize < block.dataSize) {
        return false;
    }

    // 读取数据
    memcpy(buffer, block.payload, block.dataSize);
    outCommandId = block.commandId;
    outDataType = block.dataType;

    // 清空block数据
    block.commandId = 0;
    block.dataSize = 0;
    block.dataType = 0;

    // 更新读游标
    pHeader->respRead++;

    return true;
}

DWORD SharedMemoryDateManager::GetShmSize() const {
    if (!IsReady()) {
        return 0;
    }
    return pHeader->shmSize;
}

DWORD SharedMemoryDateManager::GetBlockCount() const {
    if (!IsReady()) {
        return 0;
    }
    return pHeader->blockCount;
}

DWORD SharedMemoryDateManager::GetBlockSize() const {
    if (!IsReady()) {
        return 0;
    }
    return pHeader->blockSize;
}

DWORD SharedMemoryDateManager::GetVersion() const {
    if (!IsReady()) {
        return 0;
    }
    return pHeader->version;
}