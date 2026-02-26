#pragma once
#include "IResponsibilityChainMsg.h"
#include "MyParms.h"

/// <summary>
/// 执行流程职责链模式
/// </summary>
class ICoreExecator
{
public:
	virtual bool Execate(IResponsibilityChainMsg& msg, MyParams params) = 0;
	virtual ICoreExecator GetNextHandle() = 0;
	virtual bool setNextHandle(ICoreExecator next) = 0;
};