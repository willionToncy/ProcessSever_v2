#include "ShareMemoryMgr.h"

void ShareMemoryMgr::Init()
{
	if (this->mShareMemoryCommandMgr_ptr==nullptr)
	{
		this->mShareMemoryCommandMgr_ptr = new ShareMemoryCommandMgr();
	}
	if (this->mShareMemoryDateMgr_ptr==nullptr)
	{
		this->mShareMemoryDateMgr_ptr = new SharedMemoryDateManager();
	}
}