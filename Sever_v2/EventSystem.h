#pragma once
#include<iostream>
#include<map>
#include<list>
#include<string>
#include <functional>
#include<type_traits>
#include<atomic>
#include"IListener.h"
#include"MyParms.h"

enum EvnetCoreRunMode
{
	All=1,
	OnlyClass=2,
	OnlyFunction=3
};

// 监听器 ID 类型
using ListenerId = size_t;
constexpr ListenerId INVALID_LISTENER_ID = 0;

/// <summary>
/// 消息系统，负责事件的注册和管理,采用单例模式
/// </summary>
class EventSystem
{
private:

	/// <summary>
	/// 负责容器的初始化，不进行任何时间的添加
	/// </summary>
	EventSystem();
public:
	static EventSystem* GetInstance() 
	{
		if (mInstance==nullptr)
		{
			mInstance = new EventSystem();
		}
		return mInstance;
	}

	/// <summary>
	/// 发布消息,默认是不管函数还是对象的事件处理器都会执行
	/// </summary>
	bool PublishCoreEvent(std::string eventName, CoreMessage msg=CoreMessage(), EvnetCoreRunMode runMode = All);

	// ========== 添加监听器 ==========
	
	/// <summary>
	/// 添加对象监听器 (IListener*)
	/// </summary>
	/// <returns>监听器 ID，可用于移除</returns>
	ListenerId AddListener(std::string eventName, IListener* listener) {
		if (listener == nullptr) {
			return INVALID_LISTENER_ID;
		}
		ListenerId id = nextId++;
		ListenerClassMap[eventName].push_back({id, listener});
		return id;
	}

	/// <summary>
	/// 添加函数监听器 (支持 lambda, std::function, 函数指针等所有可调用对象)
	/// </summary>
	/// <returns>监听器 ID，可用于移除</returns>
	template<typename F,
		typename = std::enable_if_t<
			std::is_invocable_v<F, CoreMessage> &&
			!std::is_same_v<std::decay_t<F>, IListener*>
		>>
	ListenerId AddListener(std::string eventName, F&& listener) {
		ListenerId id = nextId++;
		ListenerFunctionMap[eventName].push_back({
			id, 
			std::function<void(CoreMessage)>(std::forward<F>(listener))
		});
		return id;
	}

	// ========== 移除监听器 ==========

	/// <summary>
	/// 通过 ID 移除监听器 (支持对象和函数监听器)
	/// </summary>
	bool RemoveListener(ListenerId id) {
		// 在类监听器中查找
		for (auto& pair : ListenerClassMap) {
			auto& list = pair.second;
			for (auto it = list.begin(); it != list.end(); ++it) {
				if (it->id == id) {
					list.erase(it);
					return true;
				}
			}
		}
		// 在函数监听器中查找
		for (auto& pair : ListenerFunctionMap) {
			auto& list = pair.second;
			for (auto it = list.begin(); it != list.end(); ++it) {
				if (it->id == id) {
					list.erase(it);
					return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// 通过地址移除对象监听器 (仅支持 IListener*)
	/// </summary>
	bool RemoveListener(IListener* listener) {
		if (listener == nullptr) {
			return false;
		}
		for (auto& pair : ListenerClassMap) {
			auto& list = pair.second;
			for (auto it = list.begin(); it != list.end(); ++it) {
				if (it->listener == listener) {
					list.erase(it);
					return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// 从指定事件中通过地址移除对象监听器
	/// </summary>
	bool RemoveListener(std::string eventName, IListener* listener) {
		if (listener == nullptr) {
			return false;
		}
		auto it = ListenerClassMap.find(eventName);
		if (it != ListenerClassMap.end()) {
			auto& list = it->second;
			for (auto iter = list.begin(); iter != list.end(); ++iter) {
				if (iter->listener == listener) {
					list.erase(iter);
					return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// 移除事件 (移除该事件的所有监听器)
	/// </summary>
	bool RemoveEvent(std::string eventName);

private:
	// 类监听器条目（带 ID）
	struct ClassListenerEntry {
		ListenerId id;
		IListener* listener;
	};

	// 函数监听器条目（带 ID）
	struct FunctionListenerEntry {
		ListenerId id;
		std::function<void(CoreMessage)> func;
	};

	// ID 生成器
	std::atomic<ListenerId> nextId{ 1 };

	/// <summary>
	/// 监听对象的容器
	/// </summary>
	std::map<std::string, std::list<ClassListenerEntry>> ListenerClassMap;

	/// <summary>
	/// 监听函数的容器
	/// </summary>
	std::map<std::string, std::list<FunctionListenerEntry>> ListenerFunctionMap;
public:
	static EventSystem* mInstance;
};

