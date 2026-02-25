#include <iostream>
#include <string>
#include <windows.h>
#include "CoreMain.h"

int main()
{
    std::cout << "=== CoreMain 共享内存测试 ===" << std::endl;
    
    // 测试不同的共享内存名称
    std::wstring testName = L"Global\\MyTestSharedMemory";
    std::wcout << L"测试共享内存名称: " << testName << std::endl;
    
    CoreMain core;
    
    std::cout << "\n1. 尝试初始化核心..." << std::endl;
    if (core.InitializeCore(testName, 1024, 100)) {
        std::cout << "✓ 核心初始化成功" << std::endl;
        
        if (core.IsCoreReady()) {
            std::cout << "✓ 核心已就绪" << std::endl;
            
            SharedMemoryManager& manager = core.GetMemoryManager();
            if (manager.IsReady()) {
                std::cout << "✓ 共享内存管理器就绪" << std::endl;
                std::cout << "内存大小: " << manager.GetShmSize() << " 字节" << std::endl;
                std::cout << "块数量: " << manager.GetBlockCount() << std::endl;
                std::cout << "块大小: " << manager.GetBlockSize() << " 字节" << std::endl;
            }
        }
        
        std::cout << "\n2. 关闭核心..." << std::endl;
        core.ShutdownCore();
        std::cout << "✓ 核心关闭完成" << std::endl;
        
    } else {
        std::cout << "✗ 核心初始化失败" << std::endl;
        
        // 尝试使用本地名称（非全局）
        std::wstring localName = L"MyLocalTestSharedMemory";
        std::wcout << L"\n3. 尝试本地共享内存名称: " << localName << std::endl;
        
        if (core.InitializeCore(localName, 1024, 100)) {
            std::cout << "✓ 本地共享内存初始化成功" << std::endl;
            core.ShutdownCore();
        } else {
            std::cout << "✗ 本地共享内存初始化也失败" << std::endl;
        }
    }
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    std::cout << "按任意键退出...";
    std::cin.get();
    return 0;
}