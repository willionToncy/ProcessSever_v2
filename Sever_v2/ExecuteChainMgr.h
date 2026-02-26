#pragma once
#include "ICoreExecator.h"
#include<iostream>
#include<map>

//执行链管理器，负责管理执行链的注册管理
class ExecuteChainMgr
{
public:
	static ExecuteChainMgr* GetInstance();

	/// <summary>
	/// 注册执行器，如果存在将返回false
	/// </summary>
	/// <param name="commandName"></param>
	/// <param name="executor"></param>
	/// <returns></returns>
	bool RegisterExecutor(const std::string& commandName, ICoreExecator* executor);

	/// <summary>
	/// 设置执行器，如果失败将返回false
	/// </summary>
	/// <param name="commandName"></param>
	/// <param name="executor"></param>
	/// <returns></returns>
	bool SetExecutor(const std::string& commandName, ICoreExecator* executor);
	
	/// <summary>
	/// 获得执行器
	/// </summary>
	/// <param name="commandName"></param>
	/// <returns></returns>
	ICoreExecator* GetExecutor(const std::string& commandName) const;

	/// <summary>
	/// 移除执行器
	/// </summary>
	/// <param name="commandName"></param>
	/// <returns></returns>
	bool RemoveExecutor(const std::string& commandName);

	/// <summary>
	/// 是否存在相应指令的执行器
	/// </summary>
	/// <param name="commandName"></param>
	/// <returns></returns>
	bool HasExecutor(const std::string& commandName) const;

	/// <summary>
	/// 清除所以指令执行器
	/// </summary>
	void ClearAllExecutors();

private:
	std::map <std::string, ICoreExecator> mCoreExecatorChainMap;
	static ExecuteChainMgr* M_INSTANCE;
};

