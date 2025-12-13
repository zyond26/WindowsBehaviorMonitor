#pragma once

#include <Windows.h>
#include <map>
#include <string>

struct ProcessInfo
{
    DWORD pid = 0;
    std::wstring processName;
    DWORD parentPid = 0;
    ULONGLONG creationTime = 0;
};

class ProcessManager
{
public:
    using ProcessMap = std::map<DWORD, ProcessInfo>;

    static bool EnableSeDebugPrivilege();

    ProcessMap GetRunningProcesses();
    std::wstring ScanProcessMemory(DWORD pid);

    static bool EnableSeDebugPrivilege();

private:
    ULONGLONG GetProcessCreationTime(DWORD pid);
    bool IsAllowlisted(const std::wstring& processName);
    ProcessMap processes_;
};
