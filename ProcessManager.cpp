#include "ProcessManager.h"

#include <TlHelp32.h>
#include <memory>
#include <sstream>
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

bool ProcessManager::EnableSeDebugPrivilege()
{
    HANDLE rawToken = nullptr;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &rawToken))
    {
        return false;
    }

    UniqueHandle token(rawToken, &::CloseHandle);

    LUID luid{};
    if (!::LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid))
    {
        return false;
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!::AdjustTokenPrivileges(token.get(), FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
    {
        return false;
    }

    return ::GetLastError() != ERROR_NOT_ALL_ASSIGNED;
}

std::wstring ProcessManager::ScanProcessMemory(DWORD pid)
{
    HANDLE rawProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!rawProcess)
    {
        return L"";
    }

    UniqueHandle process(rawProcess, &::CloseHandle);

    SYSTEM_INFO sysInfo{};
    ::GetSystemInfo(&sysInfo);

    MEMORY_BASIC_INFORMATION mbi{};
    LPVOID address = sysInfo.lpMinimumApplicationAddress;
    LPVOID maxAddress = sysInfo.lpMaximumApplicationAddress;

    while (address < maxAddress &&
           ::VirtualQueryEx(process.get(), address, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        if (mbi.State == MEM_COMMIT &&
            mbi.Type == MEM_PRIVATE &&
            mbi.Protect == PAGE_EXECUTE_READWRITE)
        {
            std::wostringstream warning;
            warning << L"Suspicious memory region detected: BaseAddress=0x"
                    << std::hex << reinterpret_cast<ULONG_PTR>(mbi.BaseAddress)
                    << L", RegionSize=0x" << mbi.RegionSize;
            return warning.str();
        }

        LPVOID nextAddress = static_cast<LPBYTE>(mbi.BaseAddress) + mbi.RegionSize;
        if (nextAddress <= address)
        {
            break;
        }
        address = nextAddress;
    }

    return L"";
}
