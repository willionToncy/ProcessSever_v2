#pragma once

#include <windows.h>
#include <string>
#include <atomic>

// 共享内存数据头部结构 (64字节对齐)
struct SharedMemoryDateHeader {
    DWORD shmSize;      // 0-3: 总大小
    DWORD blockCount;   // 4-7: 每个通道的block数量
    DWORD blockSize;    // 8-11: 每个block固定大小
    DWORD version;      // 12-15: 协议版本
    DWORD reqWrite;     // 16-19: 请求通道写游标
    DWORD reqRead;      // 20-23: 请求通道读游标
    DWORD respWrite;    // 24-27: 响应通道写游标
    DWORD respRead;     // 28-31: 响应通道读游标
    BYTE reserved[32];  // 32-63: 保留对齐/扩展
};

// 共享内存数据头部结构 (64字节对齐)，采用追击设计，当通道头游标等于读取的游标时放弃代表命令结束，每个通道内部是环形队列
struct SharedMemoryCommandHeader {
    DWORD shmSize;      // 0-3: 总大小
    DWORD blockCount;   // 4-7: 每个通道的block数量
    DWORD blockSize;    // 8-11: 每个block固定大小
    DWORD version;      // 12-15: 协议版本
    DWORD reqWrite;     // 16-19:  请求通道命令写入游标
    DWORD reqRead;      // 20-23:: 请求通道命令读取游标
    DWORD respWrite;    // 24-27:: 响应通道命令写入游标
    DWORD respRead;     // 28-31:: 响应通道命令读取游标
    BYTE reserved[32];  // 32-63:: 保留对齐/扩展
};

// 数据Block结构
struct DateBlock {
    DWORD commandId;    // 指令ID
    DWORD dataSize;     // 数据大小
    DWORD dataType;     // 数据类型
    BYTE payload[1];    // 实际数据 (变长)
};

//Command结构
struct CommandBlock {
    uint32_t type;               //指令类型
    uint32_t dataOffset;         //数据区偏移
    uint32_t dataSize;           //数据区的大小
    uint32_t flags;              //附加控制信息，暂时不启用预留
};


// 常量定义
const DWORD SHARED_MEMORY_HEADER_SIZE = 64;  // 64字节对齐
const DWORD DEFAULT_PROTOCOL_VERSION = 1;
const DWORD MIN_BLOCK_SIZE = sizeof(DateBlock) + 256;  // 最小block大小
const DWORD MAX_BLOCK_SIZE = 65536;  // 最大block大小
const DWORD MIN_BLOCK_COUNT = 1;
const DWORD MAX_BLOCK_COUNT = 1024;