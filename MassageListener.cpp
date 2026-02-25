#include "MassageListener.h"
#include <iostream>

// 用于将this指针传递给窗口过程
static const wchar_t PROP_NAME[] = L"MassageListenerPtr";

MassageListener::MassageListener() 
    : isRunning(false)
    , isInitialized(false)
    , hwnd(nullptr) {
}

MassageListener::~MassageListener() {
    StopListener();
}

LRESULT CALLBACK MassageListener::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CREATE) {
        // 保存this指针到窗口属性
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetPropW(hwnd, PROP_NAME, lpcs->lpCreateParams);
        return 0;
    }
    
    if (uMsg == WM_APP) {
        // 从窗口属性获取this指针
        MassageListener* pThis = reinterpret_cast<MassageListener*>(GetPropW(hwnd, PROP_NAME));
        if (pThis != nullptr) {
            CommandEnem cmd = static_cast<CommandEnem>(wParam);
            auto it = pThis->callbacks.find(cmd);
            if (it != pThis->callbacks.end() && it->second != nullptr) {
                bool result = it->second();
                return result ? 1 : 0;
            }
        }
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MassageListener::MessageLoop() {
    const wchar_t CLASS_NAME[] = L"MassageListenerWindowClass";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    
    if (!RegisterClassW(&wc)) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            std::cerr << "RegisterClassW failed with error: " << error << std::endl;
            isRunning = false;
            return;
        }
    }
    
    // 创建消息窗口，将this指针传递给lpParam
    hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"MassageListener",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );
    
    if (hwnd == nullptr) {
        std::cerr << "CreateWindowExW failed with error: " << GetLastError() << std::endl;
        isRunning = false;
        return;
    }
    
    // 消息循环
    MSG msg = {};
    while (isRunning && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // 清理
    if (hwnd != nullptr) {
        RemovePropW(hwnd, PROP_NAME);
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
    UnregisterClassW(CLASS_NAME, GetModuleHandle(nullptr));
    
    isRunning = false;
}

bool MassageListener::RegisterlistenerCallBack(CommandEnem coreCommand, bool(*pCallBack)()) {
    if (pCallBack == nullptr) {
        return false;
    }
    
    // 检查命令是否有效（在CommandEnem定义范围内）
    switch (coreCommand) {
        case Invalid:
        case SystemCall:
        case UserCall:
        case SysNotigy:
        case UserNotify:
        case HasRebackDate:
        case Error:
        case Final:
        case Stop:
        case Reload:
            callbacks[coreCommand] = pCallBack;
            return true;
        default:
            return false;
    }
}

void MassageListener::Init() {
    isInitialized = true;
}

bool MassageListener::StarListener() {
    if (!isInitialized) {
        return false;
    }
    
    if (isRunning) {
        return true;
    }
    
    isRunning = true;
    
    // 启动后台线程执行消息循环
    listenerThread = std::thread(&MassageListener::MessageLoop, this);
    
    return true;
}

void MassageListener::StopListener() {
    if (isRunning) {
        isRunning = false;
        
        // 发送消息唤醒消息循环
        if (hwnd != nullptr) {
            PostMessage(hwnd, WM_QUIT, 0, 0);
        }
        
        // 等待线程结束
        if (listenerThread.joinable()) {
            listenerThread.join();
        }
    }
}
