#pragma once
#include "CommandEnem.h"
#include <string>
#include <map>
#include <thread>
#include <atomic>
#include <windows.h>

/// <summary>
/// 监听Windows的消息指令的对象
/// </summary>
class MassageListener
{
public:
	/// <summary>
	/// 构造函数
	/// </summary>
	MassageListener();

	/// <summary>
	/// 析构函数
	/// </summary>
	~MassageListener();

	/// <summary>
	/// 注册消息收到后的回调函数
	/// </summary>
	bool RegisterlistenerCallBack(CommandEnem coreCommand, bool(*pCallBack)());

	/// <summary>
	/// 初始化监听对象
	/// </summary>
	void Init();

	/// <summary>
	/// 启动对windows事件进行监听（异步，立即返回）
	/// </summary>
	bool StarListener();

	/// <summary>
	/// 停止监听
	/// </summary>
	void StopListener();

private:
	std::map<CommandEnem, bool(*)()> callbacks;
	std::atomic<bool> isRunning;
	std::atomic<bool> isInitialized;
	std::thread listenerThread;
	HWND hwnd;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void MessageLoop();
};

