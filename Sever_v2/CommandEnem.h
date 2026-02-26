#pragma once

/// <summary>
/// 系统指令集
/// </summary>
enum CommandEnem
{
	/// <summary>
	///无效指令
	/// </summary>
	Invalid = 0,

	/// <summary>
	/// 调用指令
	/// </summary>
	SystemCall = 1,
	UserCall = 2,

	/// <summary>
	/// 通知指令不需要等待对方回信
	/// </summary>
	SysNotigy = 5,
	UserNotify = 6,

	/// <summary>
	/// 调用返回指令通知对方我方已完成计算
	/// </summary>
	HasRebackDate=7,

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//内核功能指令
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	/// <summary>
	/// 自身错误发送广播
	/// </summary>
	Error=100,

	/// <summary>
	/// 自身进程终止广播
	/// </summary>
	Final=101,

	/// <summary>
	/// 对方进程停止
	/// </summary>
	Stop=102,
	
	/// <summary>
	/// 对方进程重启
	/// </summary>
	Reload=103
}; 