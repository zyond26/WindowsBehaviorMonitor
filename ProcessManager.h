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

    ProcessMap GetRunningProcesses();

private:
    ULONGLONG GetProcessCreationTime(DWORD pid);
    ProcessMap processes_;
};
