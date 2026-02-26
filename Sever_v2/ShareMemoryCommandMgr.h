#pragma once
#include"CommandEnem.h"
#include"ShareMonyDate.h"
#include<windows.h>
#include<string>

class ShareMemoryCommandMgr
{
private:
	HANDLE hMapFile;
	LPVOID pMapView;
	SharedMemoryCommandHeader* pHeader;
	CommandBlock* pRequestBlocks;
	CommandBlock* pResponseBlocks;
	std::wstring sharedMemoryName;
	bool isInitialized;

	bool CreateMapFile(const std::wstring& name, DWORD size);
	bool OpenMapFile(const std::wstring& name);
	void InitializeLayout();
	DWORD CalculateTotalSize() const;
	void Cleanup();
	bool IsReady() const;

public:
	/// <summary>
	/// 构造函数
	/// </summary>
	ShareMemoryCommandMgr();

	/// <summary>
	/// 析构函数
	/// </summary>
	~ShareMemoryCommandMgr();

	/// <summary>
	/// 初始化指令共享内存管理器
	/// </summary>
	void Init();

	/// <summary>
	/// 读取下一条指令
	/// </summary>
	CommandBlock ReadRequestNextCommand();
	CommandBlock ReadResponseNextCommand();

	/// <summary>
	/// 写入一条指令
	/// </summary>
	bool WriteRequestNextCommand(CommandBlock command);
	bool WriteResponseNextCommand(CommandBlock command);

};

