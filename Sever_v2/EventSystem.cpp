#include "EventSystem.h"

// 静态成员变量定义
EventSystem* EventSystem::mInstance = nullptr;

EventSystem::EventSystem()
{
}

bool EventSystem::PublishCoreEvent(std::string eventName, CoreMessage msg, EvnetCoreRunMode runMode)
{
	switch (runMode) {
	case EvnetCoreRunMode::All:
		{
			auto classIt = ListenerClassMap.find(eventName);
			if (classIt != ListenerClassMap.end()) {
				for (auto& entry : classIt->second) {
					if (entry.listener != nullptr) {
						entry.listener->Execate(msg);
					}
				}
			}

			auto funcIt = ListenerFunctionMap.find(eventName);
			if (funcIt != ListenerFunctionMap.end()) {
				for (auto& entry : funcIt->second) {
					if (entry.func) {
						entry.func(msg);
					}
				}
			}
		}
		break;

	case EvnetCoreRunMode::OnlyClass:
		{
			auto classIt = ListenerClassMap.find(eventName);
			if (classIt != ListenerClassMap.end()) {
				for (auto& entry : classIt->second) {
					if (entry.listener != nullptr) {
						entry.listener->Execate(msg);
					}
				}
			}
		}
		break;

	case EvnetCoreRunMode::OnlyFunction:
		{
			auto funcIt = ListenerFunctionMap.find(eventName);
			if (funcIt != ListenerFunctionMap.end()) {
				for (auto& entry : funcIt->second) {
					if (entry.func) {
						entry.func(msg);
					}
				}
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

bool EventSystem::RemoveEvent(std::string eventName)
{
	bool removed = false;

	if (ListenerClassMap.erase(eventName) > 0) {
		removed = true;
	}

	if (ListenerFunctionMap.erase(eventName) > 0) {
		removed = true;
	}

	return removed;
}