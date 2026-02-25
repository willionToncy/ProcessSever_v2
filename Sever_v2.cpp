// Sever_v2.cpp : 共享内存测试程序
//

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
#include "CoreMain.h"
#include "SharedMemoryManager.h"
#include "MyParms.h"
#include "MassageListener.h"

void TestSharedMemory() {
    try {
        std::cout << "===========================================" << std::endl;
        std::cout << "    共享内存系统测试" << std::endl;
        std::cout << "===========================================" << std::endl;

        CoreMain coreMain;
        coreMain.InitializeCore(L"MySeverCore", 512, 100);
        if (!coreMain.IsCoreReady()) 
        {
            std::cout << "内核启动失败" << std::endl;
            coreMain.ShutdownCore();
            return;
        }
        
        SharedMemoryManager& shm =coreMain.GetMemoryManager();
        
        
        // 测试数据
        struct TestData {
            int id;
            char name[32];
            double value;
        };
        
        TestData testData = {12345, "测试数据", 3.14159};
        
        // 写入请求通道
        if (shm.WriteRequest(&testData, sizeof(TestData), 100, 1)) {
            std::cout << "   写入请求数据成功" << std::endl;
        } else {
            std::cout << "   写入请求数据失败" << std::endl;
        }
        
        // 写入响应通道
        if (shm.WriteResponse(&testData, sizeof(TestData), 200, 2)) {
            std::cout << "   写入响应数据成功" << std::endl;
        } else {
            std::cout << "   写入响应数据失败" << std::endl;
        }
        
        // 测试读取操作
        std::cout << "\n4. 测试读取操作..." << std::endl;
        
        TestData readData;
        DWORD commandId, dataType;
        
        // 从请求通道读取
        if (shm.ReadRequest(&readData, sizeof(TestData), commandId, dataType)) {
            std::cout << "   从请求通道读取成功:" << std::endl;
            std::cout << "     命令ID: " << commandId << std::endl;
            std::cout << "     数据类型: " << dataType << std::endl;
            std::cout << "     ID: " << readData.id << std::endl;
            std::cout << "     名称: " << readData.name << std::endl;
            std::cout << "     值: " << readData.value << std::endl;
        } else {
            std::cout << "   从请求通道读取失败" << std::endl;
        }
        
        // 从响应通道读取
        if (shm.ReadResponse(&readData, sizeof(TestData), commandId, dataType)) {
            std::cout << "   从响应通道读取成功:" << std::endl;
            std::cout << "     命令ID: " << commandId << std::endl;
            std::cout << "     数据类型: " << dataType << std::endl;
            std::cout << "     ID: " << readData.id << std::endl;
            std::cout << "     名称: " << readData.name << std::endl;
            std::cout << "     值: " << readData.value << std::endl;
        } else {
            std::cout << "   从响应通道读取失败" << std::endl;
        }
        
        // 测试字符串数据
        std::cout << "\n5. 测试字符串数据..." << std::endl;
        
        const char* testString = "Hello, Shared Memory!";
        DWORD stringLen = strlen(testString) + 1;
        
        if (shm.WriteRequest(testString, stringLen, 300, 3)) {
            std::cout << "   写入字符串成功: " << testString << std::endl;
            
            char readString[256];
            if (shm.ReadRequest(readString, sizeof(readString), commandId, dataType)) {
                std::cout << "   读取字符串成功: " << readString << std::endl;
            }
        }
        
        std::cout << "\n===========================================" << std::endl;
        std::cout << "    测试完成!" << std::endl;
        std::cout << "===========================================" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生异常: " << e.what() << std::endl;
    }
}

// 测试回调函数
bool OnSystemCall() {
    std::cout << "[回调] 收到 SystemCall 消息！" << std::endl;
    return true;
}

bool OnUserCall() {
    std::cout << "[回调] 收到 UserCall 消息！" << std::endl;
    return true;
}

bool OnError() {
    std::cout << "[回调] 收到 Error 消息！" << std::endl;
    return true;
}

bool OnStop() {
    std::cout << "[回调] 收到 Stop 消息！" << std::endl;
    return true;
}

// 测试 MassageListener
void TestMassageListener() {
    std::cout << "===========================================" << std::endl;
    std::cout << "    MassageListener 消息监听测试" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 创建监听器对象
    MassageListener listener;

    // 注册回调函数
    std::cout << "1. 注册回调函数..." << std::endl;
    bool result1 = listener.RegisterlistenerCallBack(SystemCall, OnSystemCall);
    bool result2 = listener.RegisterlistenerCallBack(UserCall, OnUserCall);
    bool result3 = listener.RegisterlistenerCallBack(Error, OnError);
    bool result4 = listener.RegisterlistenerCallBack(Stop, OnStop);
    
    std::cout << "   SystemCall 注册: " << (result1 ? "成功" : "失败") << std::endl;
    std::cout << "   UserCall 注册: " << (result2 ? "成功" : "失败") << std::endl;
    std::cout << "   Error 注册: " << (result3 ? "成功" : "失败") << std::endl;
    std::cout << "   Stop 注册: " << (result4 ? "成功" : "失败") << std::endl;

    // 测试无效命令注册（应该失败）
    std::cout << "2. 测试无效命令注册（应失败）..." << std::endl;
    bool result5 = listener.RegisterlistenerCallBack(static_cast<CommandEnem>(999), OnSystemCall);
    std::cout << "   无效命令注册: " << (result5 ? "成功" : "失败（符合预期）") << std::endl;

    // 测试空回调注册（应该失败）
    std::cout << "3. 测试空回调注册（应失败）..." << std::endl;
    bool result6 = listener.RegisterlistenerCallBack(Reload, nullptr);
    std::cout << "   空回调注册: " << (result6 ? "成功" : "失败（符合预期）") << std::endl;

    // 初始化并启动监听器
    std::cout << "4. 初始化并启动监听器..." << std::endl;
    listener.Init();
    bool started = listener.StarListener();
    std::cout << "   启动结果: " << (started ? "成功" : "失败") << std::endl;

    if (started) {
        std::cout << "5. 等待监听器准备就绪..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 查找监听窗口
        HWND hwnd = FindWindowW(L"MassageListenerWindowClass", L"MassageListener");
        if (hwnd != nullptr) {
            std::cout << "   找到监听窗口，开始发送测试消息..." << std::endl;

            // 发送各种命令消息
            std::cout << "6. 发送 SystemCall 消息..." << std::endl;
            SendMessage(hwnd, WM_APP, SystemCall, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "7. 发送 UserCall 消息..." << std::endl;
            SendMessage(hwnd, WM_APP, UserCall, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "8. 发送 Error 消息..." << std::endl;
            SendMessage(hwnd, WM_APP, Error, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "9. 发送 Stop 消息..." << std::endl;
            SendMessage(hwnd, WM_APP, Stop, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::cout << "10. 发送未注册的 Final 消息（应无回调）..." << std::endl;
            SendMessage(hwnd, WM_APP, Final, 0);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
            std::cout << "   错误：未找到监听窗口！" << std::endl;
        }

        // 停止监听器
        std::cout << "11. 停止监听器..." << std::endl;
        listener.StopListener();
        std::cout << "   监听器已停止" << std::endl;
    }

    std::cout << "\n===========================================" << std::endl;
    std::cout << "    MassageListener 测试完成!" << std::endl;
    std::cout << "===========================================" << std::endl;
}

int main()
{
    // 运行 MassageListener 测试
    TestMassageListener();

    std::cout << "\n按任意键退出...";
    std::cin.get();
    return 0;
}