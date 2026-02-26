#include <iostream>
#include <string>
#include "CoreMain.h"

int main()
{
    std::cout << "CoreMain Test Started" << std::endl;
    
    CoreMain core;
    
    if (core.InitializeCore(L"Global\\CoreMainTest", 1024, 100)) {
        std::cout << "Core initialized successfully" << std::endl;
        
        if (core.IsCoreReady()) {
            std::cout << "Core is ready" << std::endl;
            
            SharedMemoryManager& manager = core.GetMemoryManager();
            if (manager.IsReady()) {
                std::cout << "Memory size: " << manager.GetShmSize() << std::endl;
                std::cout << "Block count: " << manager.GetBlockCount() << std::endl;
                std::cout << "Block size: " << manager.GetBlockSize() << std::endl;
            }
        }
        
        core.ShutdownCore();
        std::cout << "Core shutdown completed" << std::endl;
    } else {
        std::cout << "Core initialization failed" << std::endl;
    }
    
    std::cout << "Press any key to exit...";
    std::cin.get();
    return 0;
}