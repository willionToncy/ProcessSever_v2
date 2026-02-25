#pragma once

#include "ShareMonyDate.h"
#include "SharedMemoryManager.h"
#include "MassageListener.h"
#include "ExecuteSysMsgMgr.h"
#include <windows.h>
#include <string>

/**
 * 核心管理类
 * 负责共享内存区的开辟和核心启动，内核主程序，但是还没写完，边写边测试。
 */
class CoreMain
{
private:
    SharedMemoryManager memoryManager;  // 共享内存管理器
    bool isInitialized;                 // 初始化状态
    std::wstring sharedMemoryName;      // 共享内存名称
    MassageListener massageListener;    // 同步消息监听器
    ExecuteSysMsgMgr executeSysMsgMgr;  // 系统同步消息执行器


    // 内部辅助方法
    DWORD CalculateMemorySize() const;

public:
    // 构造函数和析构函数
    CoreMain();
    ~CoreMain();

    // 核心功能接口
    /**
     * 初始化核心系统
     * @param name 共享内存名称
     * @param blockSize 每个块的大小
     * @param blockCount 块的数量
     * @return 初始化是否成功
     */
    bool InitializeCore(const std::wstring& name, DWORD blockSize, DWORD blockCount);
    
    /// <summary>
    /// 核心运行开始
    /// </summary>
    void Run();

    /**
     * 关闭核心系统并释放资源
     */
    void ShutdownCore();
    
    /**
     * 检查核心是否已就绪
     * @return 核心就绪状态
     */
    bool IsCoreReady() const;

    SharedMemoryManager& GetMemoryManager();
    
    /**
     * 获取共享内存总大小
     * @return 内存大小（字节）
     */
    DWORD GetMemorySize() const;
};