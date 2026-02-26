# 进程间通信微架构设计文档

## 组件详细设计

### 1. ShareMemoryMgr

**职责**：统一管理两块共享内存的生命周期

**内部实现**：
```cpp
class ShareMemoryMgr {
private:
    SharedMemoryDateManager* mShareMemoryDateMgr_ptr;      // 数据内存管理器
    ShareMemoryCommandMgr* mShareMemoryCommandMgr_ptr;     // 指令内存管理器

public:
    ShareMemoryMgr();
    ~ShareMemoryMgr();
    void Init();                                           // 初始化两块共享内存
    ShareMemoryCommandMgr* GetShareMemoryCommandMgr();     // 获取指令管理器
    SharedMemoryDateManager* GetShareMemoryDateManager();  // 获取数据管理器
};
```

**实现要点**：
- 构造函数初始化指针为 nullptr
- Init() 分别创建两个管理器实例并调用其初始化
- 析构函数负责清理资源

---

### 2. ShareMemoryCommandMgr

**职责**：管理指令共享内存，支持环形队列读写

**数据结构**：
```cpp
struct SharedMemoryCommandHeader {
    DWORD shmSize;      // 总大小
    DWORD blockCount;   // 每个通道的block数量
    DWORD blockSize;    // 每个block固定大小
    DWORD version;      // 协议版本
    DWORD reqWrite;     // 请求通道命令写入游标
    DWORD reqRead;      // 请求通道命令读取游标
    DWORD respWrite;    // 响应通道命令写入游标
    DWORD respRead;     // 响应通道命令读取游标
    BYTE reserved[32];  // 保留对齐/扩展
};

struct CommandBlock {
    uint32_t type;               // 指令类型
    uint32_t dataOffset;         // 数据区偏移
    uint32_t dataSize;           // 数据区的大小
    uint32_t flags;              // 附加控制信息
};
```

**核心算法 - 读取下一条指令**：
```cpp
CommandBlock ShareMemoryCommandMgr::ReadRequestNextCommand() {
    // 1. 检查是否有可读数据 (reqWrite != reqRead)
    DWORD writePos = pHeader->reqWrite;
    DWORD readPos = pHeader->reqRead;
    
    if (writePos == readPos) {
        return CommandBlock{0};  // 无数据，返回空块
    }
    
    // 2. 计算读取位置（环形队列）
    CommandBlock* pBlock = &pRequestBlocks[readPos];
    CommandBlock result = *pBlock;
    
    // 3. 移动读指针
    pHeader->reqRead = (readPos + 1) % pHeader->blockCount;
    
    return result;
}
```

**核心算法 - 写入指令**：
```cpp
bool ShareMemoryCommandMgr::WriteRequestNextCommand(CommandBlock command) {
    // 1. 计算写入位置（环形队列）
    DWORD writePos = pHeader->reqWrite;
    DWORD nextPos = (writePos + 1) % pHeader->blockCount;
    
    // 2. 检查队列是否已满
    if (nextPos == pHeader->reqRead) {
        return false;  // 队列已满
    }
    
    // 3. 写入数据
    pRequestBlocks[writePos] = command;
    
    // 4. 更新写指针（使用内存屏障确保顺序）
    std::atomic_thread_fence(std::memory_order_release);
    pHeader->reqWrite = nextPos;
    
    return true;
}
```

---

### 3. SharedMemoryDateManager

**职责**：管理数据共享内存，支持变长数据读写

**核心算法 - 写入数据**：
```cpp
bool SharedMemoryDateManager::WriteRequest(const void* data, DWORD dataSize, 
                                           DWORD commandId, DWORD dataType) {
    // 1. 计算所需空间
    DWORD totalSize = sizeof(DateBlock) - 1 + dataSize;
    if (totalSize > blockSize) return false;
    
    // 2. 获取写位置
    DWORD writePos = pHeader->reqWrite;
    DWORD nextPos = (writePos + 1) % blockCount;
    
    // 3. 检查空间
    if (nextPos == pHeader->reqRead) return false;
    
    // 4. 填充块数据
    DateBlock* pBlock = &pRequestBlocks[writePos];
    pBlock->commandId = commandId;
    pBlock->dataSize = dataSize;
    pBlock->dataType = dataType;
    memcpy(pBlock->payload, data, dataSize);
    
    // 5. 更新指针
    pHeader->reqWrite = nextPos;
    return true;
}
```

---

### 4. CommadChanelMgr

**职责**：异步监视共享内存管道，触发指令读取和派发

**监听线程逻辑**：
```cpp
void CommadChanelMgr::ListenerThreadFunc() {
    while (mIsListening.load()) {
        // 获取共享内存管理器
        ShareMemoryCommandMgr* cmdMgr = mShareMemoryMgr_ptr->GetShareMemoryCommandMgr();
        if (!cmdMgr) continue;
        
        // 检查是否有数据可读
        SharedMemoryCommandHeader* pHeader = /* 获取header指针 */;
        bool hasData = (pHeader->reqWrite != pHeader->reqRead);
        
        if (hasData) {
            // 批量读取并派发
            BatchReadCommands();
        } else {
            // 无数据时短暂休眠，避免CPU空转
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
```

**批量读取逻辑**：
```cpp
void CommadChanelMgr::BatchReadCommands() {
    ShareMemoryCommandMgr* cmdMgr = mShareMemoryMgr_ptr->GetShareMemoryCommandMgr();
    
    // 循环读取直到无数据
    while (true) {
        CommandBlock cmd = cmdMgr->ReadRequestNextCommand();
        if (cmd.type == 0) break;  // 无数据了
        
        // 派发给处理器
        mDispatchMgr_ptr->DispatchCommand(cmd);
    }
}
```

**生命周期管理**：
```cpp
void CommadChanelMgr::StarListenerMemory() {
    if (mIsListening.load()) return;
    mIsListening = true;
    mListenerThread = std::thread(&CommadChanelMgr::ListenerThreadFunc, this);
}

void CommadChanelMgr::StopListenerMemory() {
    mIsListening = false;
    if (mListenerThread.joinable()) {
        mListenerThread.join();
    }
}
```

---

### 5. DispatchMgr

**职责**：定义指令派发接口（本次仅实现接口框架）

**接口定义**：
```cpp
class DispatchMgr {
public:
    DispatchMgr();
    ~DispatchMgr();
    void DispatchCommand(const CommandBlock& command);  // 派发指令接口
};
```

**实现说明**：
- 构造函数和析构函数为空实现或基础初始化
- DispatchCommand 方法为空实现，预留后续业务逻辑扩展
- 不添加任何新的对外接口

---

## 线程安全设计

### 单生产者单消费者模型

本架构假设：
- **写端**：外部进程（生产者）
- **读端**：CommadChanelMgr 监听线程（消费者）

### 内存序保证

```cpp
// 写端（生产者）
std::atomic_thread_fence(std::memory_order_release);
pHeader->reqWrite = nextPos;  // 发布数据

// 读端（消费者）
DWORD writePos = pHeader->reqWrite;
std::atomic_thread_fence(std::memory_order_acquire);
// 读取数据
```

---

## 错误处理

| 错误场景 | 处理方式 |
|---------|---------|
| 共享内存创建失败 | Init() 返回 false，记录错误 |
| 队列已满 | Write 返回 false，由调用方处理 |
| 队列空读 | Read 返回空块 (type=0) |
| 线程启动失败 | StarListenerMemory 返回失败状态 |

---

## 性能考虑

1. **批量读取**：CommadChanelMgr 一次读取所有可用指令，减少系统调用
2. **无锁设计**：单生产者单消费者场景下，使用原子操作替代锁
3. **自适应休眠**：无数据时短暂休眠，平衡延迟和CPU占用
