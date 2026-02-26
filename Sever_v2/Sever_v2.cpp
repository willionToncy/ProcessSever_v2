// Sever_v2.cpp : 共享内存测试程序
//

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
#include "CoreMain.h"
#include "SharedMemoryDateManager.h"
#include "MyParms.h"
#include "MassageListener.h"
#include "EventSystem.h"
#include "ShareMemoryCommandMgr.h"

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
        
        SharedMemoryDateManager& shm =coreMain.GetMemoryManager();
        
        
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

//测试消息系统
void TestCoreMassage()
{
    EventSystem* eventSystem=EventSystem::GetInstance();

    // 创建监听器
    class MyListener : public IListener {
    public:
        void Execate() override {}
        void Execate(CoreMessage msg) override {
            std::cout << "Listener: " << msg.toString() << std::endl;
            std::cout << "Listener_index1:" << msg.getParam<std::string>(0) << std::endl;
        }
    };

    MyListener listener1, listener2;

    // 添加对象监听器
    eventSystem->AddListener("LOGIN", &listener1);
    eventSystem->AddListener("LOGIN", &listener2);

    // 添加函数监听器 - 支持多种写法：
    
    // 1. Lambda (直接写，自动支持)
    ListenerId id3 = eventSystem->AddListener("LOGIN", [](CoreMessage msg) {
        std::cout << "Lambda: " << msg.toString() << std::endl;
    });
    
    // 2. std::function
    std::function<void(CoreMessage)> func = [](CoreMessage msg) {
        std::cout << "std::function: " << msg.toString() << std::endl;
    };
    ListenerId id4 = eventSystem->AddListener("LOGIN", func);
    
    // 3. 函数指针
    void (*funcPtr)(CoreMessage) = [](CoreMessage msg) {
        std::cout << "FuncPtr: " << msg.toString() << std::endl;
    };
    ListenerId id5 = eventSystem->AddListener("LOGIN", funcPtr);
    std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    // 发布消息（默认执行所有）
    eventSystem->PublishCoreEvent("LOGIN", CoreMessage("username123", 12345));
    std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    // 只执行对象监听器
    eventSystem->PublishCoreEvent("LOGIN", CoreMessage("user456", 789), EvnetCoreRunMode::OnlyClass);
    std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    // 只执行函数监听器
    eventSystem->PublishCoreEvent("LOGIN", CoreMessage("user789", 101112), EvnetCoreRunMode::OnlyFunction);

    // 移除监听器的多种方式：
    
    // 方式1：通过地址移除 IListener* (推荐)
    eventSystem->RemoveListener(&listener1);  // 从所有事件中移除
    eventSystem->RemoveListener("LOGIN", &listener2);  // 从指定事件中移除
    
    // 方式2：通过 ID 移除 (适用于函数监听器)
    eventSystem->RemoveListener(id3);  // 移除 lambda

    std::cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;

    eventSystem->PublishCoreEvent("LOGIN", CoreMessage("user789", 101112), EvnetCoreRunMode::OnlyFunction);
    // 移除整个事件
    eventSystem->RemoveEvent("LOGIN");
   
}

// 测试指令共享内存管理器
void TestShareMemoryCommandMgr()
{
    std::cout << "===========================================" << std::endl;
    std::cout << "    ShareMemoryCommandMgr Test" << std::endl;
    std::cout << "===========================================" << std::endl;

    // 创建指令管理器
    ShareMemoryCommandMgr cmdMgr;

    // 1. 初始化
    std::cout << "1. Init Share Memory of command" << std::endl;
    cmdMgr.Init();
    std::cout << "   Init Complete" << std::endl;

    // 2. 测试写入请求指令
    std::cout << "\n2. Test write Requese Command ..." << std::endl;
    
    CommandBlock cmd1;
    cmd1.type = SystemCall;
    cmd1.dataOffset = 0;
    cmd1.dataSize = 100;
    cmd1.flags = 0;
    
    CommandBlock cmd2;
    cmd2.type = UserCall;
    cmd2.dataOffset = 100;
    cmd2.dataSize = 200;
    cmd2.flags = 0;

    bool writeResult1 = cmdMgr.WriteRequestNextCommand(cmd1);
    bool writeResult2 = cmdMgr.WriteRequestNextCommand(cmd2);
    
    std::cout << "Write systemCall Command:" << (writeResult1 ? "success" : "failture") << std::endl;
    std::cout << "Write UserCall Command:" << (writeResult2 ? "success" : "failture") << std::endl;

    // 3. 测试读取请求指令
    std::cout << "\n3.Test Read Comamand..." << std::endl;
    
    CommandBlock readCmd1 = cmdMgr.ReadRequestNextCommand();
    if (readCmd1.type != Invalid) {
        std::cout << "Read Command success:" << std::endl;
        std::cout << "Command Kind:" << readCmd1.type << " (SystemCall=" << SystemCall << ")" << std::endl;
        std::cout << "Data Offset:" << readCmd1.dataOffset << std::endl;
        std::cout << "Data size:" << readCmd1.dataSize << std::endl;
    } else {
        std::cout << "Read Command failture" << std::endl;
    }

    CommandBlock readCmd2 = cmdMgr.ReadRequestNextCommand();
    if (readCmd2.type != Invalid) {
        std::cout << "Read Second Command success:" << std::endl;
        std::cout << "Command Kind:" << readCmd2.type << " (UserCall=" << UserCall << ")" << std::endl;
    }

    // 4. 测试空队列读取
    std::cout << "\n4. Test read empty command Queue..." << std::endl;
    CommandBlock emptyCmd = cmdMgr.ReadRequestNextCommand();
    std::cout << "Result of read empty command Queue..." << (emptyCmd.type == Invalid ? "return Invalid (As expected)":"Error") << std::endl;

    // 5. 测试响应通道
    std::cout << "\n5. Test Response Chanel" << std::endl;
    
    CommandBlock respCmd;
    respCmd.type = HasRebackDate;
    respCmd.dataOffset = 0;
    respCmd.dataSize = 50;
    respCmd.flags = 0;

    bool respWriteResult = cmdMgr.WriteResponseNextCommand(respCmd);
    std::cout << "Write Reasponse Command HasRebackDate:" << (respWriteResult ? "success" : "failture") << std::endl;

    CommandBlock respReadCmd = cmdMgr.ReadResponseNextCommand();
    if (respReadCmd.type != Invalid) {
        std::cout << "Read Command success" << std::endl;
        std::cout << "  comand kind: " << respReadCmd.type << " (HasRebackDate=" << HasRebackDate << ")" << std::endl;
    }

    // 6. 测试各种指令类型
    std::cout << "\n6. Test different commad of command..." << std::endl;
    
    CommandBlock testCmds[] = {
        {Error, 0, 0, 0},
        {Final, 0, 0, 0},
        {Stop, 0, 0, 0},
        {Reload, 0, 0, 0},
        {SysNotigy, 0, 0, 0},
        {UserNotify, 0, 0, 0}
    };

    for (int i = 0; i < 6; i++) {
        bool result = cmdMgr.WriteRequestNextCommand(testCmds[i]);
        std::cout << " Write command type " << testCmds[i].type << ": " << (result ? "success" : "failture") << std::endl;
    }

    // 读取并验证
    std::cout << "\n       Read And Test All Command..." << std::endl;
    int count = 0;
    CommandBlock cmd;
    while ((cmd = cmdMgr.ReadRequestNextCommand()).type != Invalid) {
        std::cout << "  read Command[" << count << "]: type=" << cmd.type << std::endl;
        count++;
    }
    std::cout << "   Read total number of Command is" << count << std::endl;

    std::cout << "\n===========================================" << std::endl;
    std::cout << "    ShareMemoryCommandMgr Test complete!" << std::endl;
    std::cout << "===========================================" << std::endl;
}

int main()
{
    TestShareMemoryCommandMgr();

    std::cout << "\nplease enter any Key to exit";
    std::cin.get();
    return 0;
}
