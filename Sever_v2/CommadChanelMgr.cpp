#include "CommadChanelMgr.h"

CommadChanelMgr::CommadChanelMgr()
    : mShareMemoryMgr_ptr(nullptr)
    , mDispatchMgr_ptr(nullptr)
    , mIsListening(false)
{
}

CommadChanelMgr::~CommadChanelMgr()
{
    StopListenerMemory();

    if (mDispatchMgr_ptr != nullptr)
    {
        delete mDispatchMgr_ptr;
        mDispatchMgr_ptr = nullptr;
    }

    if (mShareMemoryMgr_ptr != nullptr)
    {
        delete mShareMemoryMgr_ptr;
        mShareMemoryMgr_ptr = nullptr;
    }
}

void CommadChanelMgr::Init()
{
    mShareMemoryMgr_ptr = new ShareMemoryMgr();
    mDispatchMgr_ptr = new DispatchMgr();

    mShareMemoryMgr_ptr->Init();
}

void CommadChanelMgr::StarListenerMemory()
{
    if (mIsListening.load())
        return;

    mIsListening = true;
    mListenerThread = std::thread(&CommadChanelMgr::ListenerThreadFunc, this);
}

void CommadChanelMgr::StopListenerMemory()
{
    mIsListening = false;

    if (mListenerThread.joinable())
    {
        mListenerThread.join();
    }
}

void CommadChanelMgr::ListenerThreadFunc()
{
    while (mIsListening.load())
    {
        if (mShareMemoryMgr_ptr == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        ShareMemoryCommandMgr* cmdMgr = mShareMemoryMgr_ptr->GetShareMemoryCommandMgr();
        if (cmdMgr == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        CommandBlock cmd = cmdMgr->ReadRequestNextCommand();
        if (cmd.type != 0)
        {
            mDispatchMgr_ptr->DispatchCommand(cmd);
            BatchReadCommands();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void CommadChanelMgr::BatchReadCommands()
{
    if (mShareMemoryMgr_ptr == nullptr || mDispatchMgr_ptr == nullptr)
        return;

    ShareMemoryCommandMgr* cmdMgr = mShareMemoryMgr_ptr->GetShareMemoryCommandMgr();
    if (cmdMgr == nullptr)
        return;

    while (true)
    {
        CommandBlock cmd = cmdMgr->ReadRequestNextCommand();
        if (cmd.type == 0)
        {
            break;
        }

        mDispatchMgr_ptr->DispatchCommand(cmd);
    }
}
