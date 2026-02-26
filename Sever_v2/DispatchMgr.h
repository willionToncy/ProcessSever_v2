#pragma once
#include "ShareMemoryCommandMgr.h"
#include "CommandEnem.h"

class DispatchMgr
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	DispatchMgr();

	/// <summary>
	/// 析构函数
	/// </summary>
	~DispatchMgr();

	/// <summary>
	/// 派发指令
	/// </summary>
	/// <param name="command">指令块</param>
	void DispatchCommand(const CommandBlock& coreCommand);


private:

	
};

