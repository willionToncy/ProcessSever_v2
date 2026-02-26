#pragma once
#include"SharedMemoryDateManager.h"
#include"ShareMemoryCommandMgr.h"

/// <summary>
/// 统一管理并初始化两块共享内存.
/// </summary>
class ShareMemoryMgr
{
private:
	SharedMemoryDateManager* mShareMemoryDateMgr_ptr;
	ShareMemoryCommandMgr* mShareMemoryCommandMgr_ptr;
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	ShareMemoryMgr();

	/// <summary>
	/// 析构函数
	/// </summary>
	~ShareMemoryMgr();

	/// <summary>
	/// 初始化共享内存
	/// </summary>
	void Init();

	/// <summary>
	/// 获取指令共享内存管理器
	/// </summary>
	/// <returns>指令共享内存管理器指针</returns>
	ShareMemoryCommandMgr* GetShareMemoryCommandMgr();

	/// <summary>
	/// 获取数据共享内存管理器
	/// </summary>
	/// <returns>数据共享内存管理器指针</returns>
	SharedMemoryDateManager* GetShareMemoryDateManager();
};

