#include "CoreMain.h"
#include <algorithm>

CoreMain::CoreMain() 
    : isInitialized(false) {
}

CoreMain::~CoreMain() {
   
}

bool CoreMain::InitializeCore(const std::wstring& name, DWORD blockSize, DWORD blockCount) {
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
    
    // 通过SharedMemoryManager初始化共享内存
    if (!memoryManager.Initialize(name, blockSize, blockCount)) {
        return false;
    }

    

    isInitialized = true;
    return true;
}

SharedMemoryManager& CoreMain::GetMemoryManager() {
    return memoryManager;
}


DWORD CoreMain::GetMemorySize() const {
    if (!isInitialized || !memoryManager.IsReady()) {
        return 0;
    }
    return memoryManager.GetShmSize();
}



DWORD CoreMain::CalculateMemorySize() const {
    // 计算总大小：头部 + 2 * (block数量 * block大小)
    // 确保64字节对齐
    DWORD blockSize = 1024;
    DWORD blockCount = 100;
    DWORD size = SHARED_MEMORY_HEADER_SIZE + 2 * blockSize * blockCount;
    return (size + 63) / 64 * 64; // 64字节对齐
}


void CoreMain::ShutdownCore() 
{
     memoryManager.Cleanup();
     isInitialized = false;
};


bool CoreMain::IsCoreReady() const 
{
    return isInitialized && memoryManager.IsReady();
};

