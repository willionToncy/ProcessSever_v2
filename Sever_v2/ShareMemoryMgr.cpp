#include "ShareMemoryMgr.h"

ShareMemoryMgr::ShareMemoryMgr()
    : mShareMemoryDateMgr_ptr(nullptr)
    , mShareMemoryCommandMgr_ptr(nullptr)
{
}

ShareMemoryMgr::~ShareMemoryMgr()
{
    if (mShareMemoryDateMgr_ptr != nullptr)
    {
        delete mShareMemoryDateMgr_ptr;
        mShareMemoryDateMgr_ptr = nullptr;
    }

    if (mShareMemoryCommandMgr_ptr != nullptr)
    {
        delete mShareMemoryCommandMgr_ptr;
        mShareMemoryCommandMgr_ptr = nullptr;
    }
}

void ShareMemoryMgr::Init()
{
    mShareMemoryDateMgr_ptr = new SharedMemoryDateManager();
    mShareMemoryCommandMgr_ptr = new ShareMemoryCommandMgr();

    mShareMemoryDateMgr_ptr->Initialize(L"SharedMemoryDate", 1024, 64);
    mShareMemoryCommandMgr_ptr->Init();
}

ShareMemoryCommandMgr* ShareMemoryMgr::GetShareMemoryCommandMgr()
{
    return mShareMemoryCommandMgr_ptr;
}

SharedMemoryDateManager* ShareMemoryMgr::GetShareMemoryDateManager()
{
    return mShareMemoryDateMgr_ptr;
}
