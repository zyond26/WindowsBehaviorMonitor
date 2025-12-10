#include "ProcessManager.h"

#include <TlHelp32.h>
#include <memory>
#include <utility>

namespace
{
    using HandleDeleter = decltype(&::CloseHandle);
    using UniqueHandle = std::unique_ptr<void, HandleDeleter>;
}

ProcessManager::ProcessMap ProcessManager::GetRunningProcesses()
{
    ProcessMap snapshotState;

    HANDLE rawSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (rawSnapshot == INVALID_HANDLE_VALUE)
    {
        return snapshotState;
    }

    UniqueHandle snapshot(rawSnapshot, &::CloseHandle);

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (::Process32First(snapshot.get(), &entry))
    {
        do
        {
            ProcessInfo info{};
            info.pid = entry.th32ProcessID;
            info.parentPid = entry.th32ParentProcessID;
            info.processName = entry.szExeFile;
            info.creationTime = GetProcessCreationTime(info.pid);
            if (info.creationTime == 0)
            {
                const auto cached = processes_.find(info.pid);
                if (cached != processes_.end())
                {
                    info.creationTime = cached->second.creationTime;
                }
            }

            snapshotState.emplace(info.pid, std::move(info));
        }
        while (::Process32Next(snapshot.get(), &entry));
    }

    processes_ = snapshotState;
    return processes_;
}

ULONGLONG ProcessManager::GetProcessCreationTime(DWORD pid)
{
    HANDLE rawProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!rawProcess)
    {
        return 0;
    }

    UniqueHandle process(rawProcess, &::CloseHandle);

    FILETIME creation{}, exit{}, kernel{}, user{};
    if (!::GetProcessTimes(process.get(), &creation, &exit, &kernel, &user))
    {
        return 0;
    }

    ULARGE_INTEGER creationTime{};
    creationTime.LowPart = creation.dwLowDateTime;
    creationTime.HighPart = creation.dwHighDateTime;

    return creationTime.QuadPart;
}
