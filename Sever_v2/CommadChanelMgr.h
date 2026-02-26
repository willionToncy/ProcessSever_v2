#pragma once
#include "ShareMemoryMgr.h"

/// <summary>
/// 负责系统指令通道的监听，以组合关系掌管消息管理器和指令派发执行器
/// </summary>
class CommadChanelMgr
{
public:
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
	void StarListenerMemory();

private:
	/// <summary>
	/// 共享内存管理器
	/// </summary>
	ShareMemoryMgr* mShareMemoryMgr_ptr;
};

