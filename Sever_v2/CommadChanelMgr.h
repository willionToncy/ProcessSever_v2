#pragma once
#include "ShareMemoryMgr.h"
#include "DispatchMgr.h"
#include <thread>
#include <atomic>
#include <chrono>

/// <summary>
/// 负责系统指令通道的监听，以组合关系掌管消息管理器和指令派发执行器
/// </summary>
class CommadChanelMgr
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	CommadChanelMgr();

	/// <summary>
	/// 析构函数
	/// </summary>
	~CommadChanelMgr();

	/// <summary>
	/// 初始化函数
	/// </summary>
	void Init();

	/// <summary>
	/// 开始进行共享内存的指令的监听
	/// </summary>
	void StarListenerMemory();

	/// <summary>
	/// 结束对共享内存的指令的监听
	/// </summary>
	void StopListenerMemory();

private:
	/// <summary>
	/// 共享内存管理器
	/// </summary>
	ShareMemoryMgr* mShareMemoryMgr_ptr;

	/// <summary>
	/// 派发管理器
	/// </summary>
	DispatchMgr* mDispatchMgr_ptr;

	/// <summary>
	/// 监听线程
	/// </summary>
	std::thread mListenerThread;

	/// <summary>
	/// 监听标志
	/// </summary>
	std::atomic<bool> mIsListening;

	/// <summary>
	/// 监听线程函数
	/// </summary>
	void ListenerThreadFunc();

	/// <summary>
	/// 批量读取指令
	/// </summary>
	void BatchReadCommands();
};

