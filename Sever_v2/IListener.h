#pragma once
#include "CoreMessage.h"

class IListener
{
public:
	virtual void Execate() = 0;
	
	virtual void Execate(CoreMessage msg) = 0;
	
	template<typename... Args>
	void ExecateWithArgs(Args&&... args) {
		Execate(CoreMessage(std::forward<Args>(args)...));
	}
};