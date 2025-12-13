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

bool ProcessManager::EnableSeDebugPrivilege()
{
    HANDLE rawToken = nullptr;
    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &rawToken))
    {
        return false;
    }

    UniqueHandle token(rawToken, &::CloseHandle);

    LUID luid{};
    if (!::LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid))
    {
        return false;
    }

    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!::AdjustTokenPrivileges(token.get(), FALSE, &tp, sizeof(tp), nullptr, nullptr))
    {
        return false;
    }

    return ::GetLastError() == ERROR_SUCCESS;
}

bool ProcessManager::IsAllowlisted(const std::wstring& processName)
{
    // Convert to lowercase for case-insensitive comparison
    std::wstring lowerName = processName;
    for (wchar_t& c : lowerName)
    {
        c = towlower(c);
    }

    // List of known safe applications that use JIT compilation
    const std::wstring allowlist[] = {
        L"chrome.exe",
        L"msedge.exe",
        L"firefox.exe",
        L"opera.exe",
        L"brave.exe",
        L"iexplore.exe",
        L"java.exe",
        L"javaw.exe",
        L"node.exe",
        L"python.exe",
        L"pythonw.exe",
        L"dotnet.exe",
        L"teams.exe",
        L"slack.exe",
        L"discord.exe"
    };

    for (const auto& allowed : allowlist)
    {
        if (lowerName == allowed)
        {
            return true;
        }
    }

    return false;
}

std::wstring ProcessManager::ScanProcessMemory(DWORD pid)
{
    HANDLE rawProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!rawProcess)
    {
        return L"";
    }

    UniqueHandle process(rawProcess, &::CloseHandle);

    // Get process name for allowlist check
    std::wstring processName;
    const auto cached = processes_.find(pid);
    if (cached != processes_.end())
    {
        processName = cached->second.processName;
    }

    // Skip scanning if process is allowlisted
    if (IsAllowlisted(processName))
    {
        return L"";
    }

    std::wstring warnings;
    MEMORY_BASIC_INFORMATION mbi{};
    LPVOID address = nullptr;

    while (::VirtualQueryEx(process.get(), address, &mbi, sizeof(mbi)) == sizeof(mbi))
    {
        // Detection: MEM_COMMIT + MEM_PRIVATE + PAGE_EXECUTE_READWRITE
        if (mbi.State == MEM_COMMIT &&
            mbi.Type == MEM_PRIVATE &&
            mbi.Protect == PAGE_EXECUTE_READWRITE)
        {
            wchar_t buffer[256]{};
            swprintf_s(buffer, L"Suspicious region: Base=0x%p, Size=0x%llX\n",
                       mbi.BaseAddress, static_cast<unsigned long long>(mbi.RegionSize));
            warnings += buffer;
        }

        // Move to the next region
        address = reinterpret_cast<LPVOID>(
            reinterpret_cast<ULONG_PTR>(mbi.BaseAddress) + mbi.RegionSize);
    }

    return warnings;
}
