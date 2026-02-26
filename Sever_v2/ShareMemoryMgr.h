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
	/// 初始化共享内存
	/// </summary>
	void Init();
};

