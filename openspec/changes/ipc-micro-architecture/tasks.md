# 进程间通信微架构实现任务

## 任务列表

### 阶段 1: ShareMemoryMgr 实现

- [x] **task-1**: 实现 ShareMemoryMgr 构造函数
  - 初始化 mShareMemoryDateMgr_ptr 和 mShareMemoryCommandMgr_ptr 为 nullptr
  - 文件: `ShareMemoryMgr.cpp`

- [x] **task-2**: 实现 ShareMemoryMgr Init() 方法
  - 创建 SharedMemoryDateManager 实例
  - 创建 ShareMemoryCommandMgr 实例
  - 调用两者的 Init() 方法
  - 文件: `ShareMemoryMgr.cpp`

- [x] **task-3**: 实现 ShareMemoryMgr 析构函数
  - 释放两个管理器实例
  - 文件: `ShareMemoryMgr.cpp`

- [x] **task-4**: 实现 ShareMemoryMgr 获取方法
  - 实现 GetShareMemoryCommandMgr()
  - 实现 GetShareMemoryDateManager()
  - 文件: `ShareMemoryMgr.cpp`

---

### 阶段 2: ShareMemoryCommandMgr 实现

- [x] **task-5**: 实现 ShareMemoryCommandMgr 构造函数和析构函数
  - 初始化所有成员变量
  - 析构函数调用 Cleanup()
  - 文件: `ShareMemoryCommandMgr.cpp`

- [x] **task-6**: 实现 ShareMemoryCommandMgr 内部辅助方法
  - 实现 CreateMapFile()
  - 实现 OpenMapFile()
  - 实现 CalculateTotalSize()
  - 实现 Cleanup()
  - 实现 IsReady()
  - 文件: `ShareMemoryCommandMgr.cpp`

- [x] **task-7**: 实现 ShareMemoryCommandMgr Init() 和 InitializeLayout()
  - 创建/打开共享内存映射文件
  - 初始化内存布局结构
  - 设置初始指针位置
  - 文件: `ShareMemoryCommandMgr.cpp`

- [x] **task-8**: 实现 ShareMemoryCommandMgr 读取方法
  - 实现 ReadRequestNextCommand()
  - 实现 ReadResponseNextCommand()
  - 处理环形队列边界
  - 文件: `ShareMemoryCommandMgr.cpp`

- [x] **task-9**: 实现 ShareMemoryCommandMgr 写入方法
  - 实现 WriteRequestNextCommand()
  - 实现 WriteResponseNextCommand()
  - 处理队列满的情况
  - 文件: `ShareMemoryCommandMgr.cpp`

---

### 阶段 3: SharedMemoryDateManager 实现

- [x] **task-10**: 实现 SharedMemoryDateManager 构造函数和析构函数
  - 初始化所有成员变量
  - 析构函数调用 Cleanup()
  - 文件: `SharedMemoryDateManager.cpp`

- [x] **task-11**: 实现 SharedMemoryDateManager 内部辅助方法
  - 实现 CreateMapFile()
  - 实现 OpenMapFile()
  - 实现 CalculateTotalSize()
  - 实现 Cleanup()
  - 文件: `SharedMemoryDateManager.cpp`

- [x] **task-12**: 实现 SharedMemoryDateManager Initialize()
  - 创建/打开共享内存映射文件
  - 初始化内存布局结构
  - 文件: `SharedMemoryDateManager.cpp`

- [x] **task-13**: 实现 SharedMemoryDateManager 写入方法
  - 实现 WriteRequest()
  - 实现 WriteResponse()
  - 处理变长数据写入
  - 文件: `SharedMemoryDateManager.cpp`

- [x] **task-14**: 实现 SharedMemoryDateManager 读取方法
  - 实现 ReadRequest()
  - 实现 ReadResponse()
  - 处理变长数据读取
  - 文件: `SharedMemoryDateManager.cpp`

- [x] **task-15**: 实现 SharedMemoryDateManager 状态查询方法
  - 实现 GetShmSize()
  - 实现 GetBlockCount()
  - 实现 GetBlockSize()
  - 实现 GetVersion()
  - 文件: `SharedMemoryDateManager.cpp`

---

### 阶段 4: CommadChanelMgr 实现

- [x] **task-16**: 实现 CommadChanelMgr 构造函数和析构函数
  - 初始化成员变量
  - 确保线程正确停止
  - 文件: `CommadChanelMgr.cpp`

- [x] **task-17**: 实现 CommadChanelMgr Init() 方法
  - 创建 ShareMemoryMgr 实例
  - 创建 DispatchMgr 实例
  - 调用 ShareMemoryMgr 的 Init()
  - 文件: `CommadChanelMgr.cpp`

- [x] **task-18**: 实现 CommadChanelMgr 监听线程函数
  - 实现 ListenerThreadFunc()
  - 循环检测 reqWrite 和 reqRead
  - 不一致时调用 BatchReadCommands()
  - 一致时短暂休眠
  - 文件: `CommadChanelMgr.cpp`

- [x] **task-19**: 实现 CommadChanelMgr 批量读取方法
  - 实现 BatchReadCommands()
  - 循环读取直到无数据
  - 调用 DispatchMgr 派发每条指令
  - 文件: `CommadChanelMgr.cpp`

- [x] **task-20**: 实现 CommadChanelMgr 生命周期控制
  - 实现 StarListenerMemory()
  - 实现 StopListenerMemory()
  - 正确处理线程启动和停止
  - 文件: `CommadChanelMgr.cpp`

---

### 阶段 5: DispatchMgr 接口实现

- [x] **task-21**: 实现 DispatchMgr 构造函数和析构函数
  - 基础初始化
  - 文件: `DispatchMgr.cpp`

- [x] **task-22**: 实现 DispatchMgr DispatchCommand() 接口
  - 空实现，预留后续扩展
  - 文件: `DispatchMgr.cpp`

---

### 阶段 6: 编译验证

- [x] **task-23**: 创建所有 .cpp 文件并添加到项目
  - 确保所有文件编译通过
  - 检查无链接错误

- [x] **task-24**: 验证接口一致性
  - 确认未添加头文件未定义的对外接口
  - 确认内部处理函数完整

## 依赖关系

```
task-1 ─┬─► task-2 ─┬─► task-3 ─┬─► task-4
        │           │           │
task-5 ─┼─► task-6 ─┼─► task-7 ─┼─► task-8 ─┬─► task-9
        │           │           │           │
task-10─┼─► task-11─┼─► task-12─┼─► task-13─┼─► task-14 ─► task-15
        │           │           │           │
task-16─┼─► task-17─┼─► task-18─┼─► task-19─┼─► task-20
        │                       │           │
task-21─┴─► task-22 ───────────┴───────────┘
                    │
                    ▼
              task-23 ─► task-24
```

## 实现顺序建议

1. **先实现 ShareMemoryCommandMgr 和 SharedMemoryDateManager**（基础组件）
2. **然后实现 ShareMemoryMgr**（组合组件）
3. **接着实现 DispatchMgr**（简单接口）
4. **最后实现 CommadChanelMgr**（依赖其他组件）
5. **最后进行编译验证**
