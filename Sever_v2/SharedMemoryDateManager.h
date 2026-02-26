#pragma once

#include "ShareMonyDate.h"
#include <windows.h>
#include <string>
#include <atomic>
#include <iostream>

/**
 * 共享内存管理类
 * 负责共享内存的创建、初始化、读写操作和资源管理
 */
class SharedMemoryDateManager
{
private:
    HANDLE hMapFile;                    // 内存映射文件句柄
    LPVOID pMapView;                    // 映射视图指针
    SharedMemoryDateHeader* pHeader;        // 头部指针
    DateBlock* pRequestBlocks;              // 请求块数组指针
    DateBlock* pResponseBlocks;             // 响应块数组指针
    std::wstring sharedMemoryName;      // 共享内存名称
    bool isInitialized;                 // 初始化状态
    DWORD totalBlockSize;               // 总block大小

    // 内部辅助方法
    bool CreateMapFile(DWORD size);
    bool OpenMapFile();
    void InitializeLayout();
    DWORD CalculateTotalSize() const;

public:
    // 构造函数和析构函数
    SharedMemoryDateManager();
    ~SharedMemoryDateManager();
    
    // 禁用拷贝构造和拷贝赋值（防止句柄重复释放）
    SharedMemoryDateManager(const SharedMemoryDateManager&) = delete;
    SharedMemoryDateManager& operator=(const SharedMemoryDateManager&) = delete;

    // 初始化和清理
    bool Initialize(const std::wstring& name, DWORD blockSize, DWORD blockCount);
    void Cleanup();
    bool IsReady() const;

    // 写入操作
    bool WriteRequest(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType);
    bool WriteResponse(const void* data, DWORD dataSize, DWORD commandId, DWORD dataType);

    // 读取操作
    bool ReadRequest(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType);
    bool ReadResponse(void* buffer, DWORD bufferSize, DWORD& outCommandId, DWORD& outDataType);

    // 状态查询
    DWORD GetShmSize() const;
    DWORD GetBlockCount() const;
    DWORD GetBlockSize() const;
    DWORD GetVersion() const;
};