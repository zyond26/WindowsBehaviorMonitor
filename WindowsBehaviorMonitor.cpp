// WindowsBehaviorMonitor.cpp : Entry point that exercises process enumeration.

#include <iostream>
#include <string>
#include <iomanip>

#include "ProcessManager.h"

int main()
{
    ProcessManager manager;
    const auto processes = manager.GetRunningProcesses();

    constexpr int kPidWidth = 8;
    constexpr int kParentPidWidth = 12;
    constexpr int kCreationWidth = 24;

    std::wcout << std::left
               << std::setw(kPidWidth) << L"PID"
               << std::setw(kParentPidWidth) << L"ParentPID"
               << std::setw(kCreationWidth) << L"CreationTimeTicks"
               << L"ProcessName" << std::endl;
    for (const auto& pair : processes)
    {
        const auto& info = pair.second;
        const std::wstring creationColumn = info.creationTime == 0
            ? L"undefined"
            : std::to_wstring(info.creationTime);
        std::wcout << std::left
            << std::setw(kPidWidth) << info.pid
            << std::setw(kParentPidWidth) << info.parentPid
            << std::setw(kCreationWidth) << creationColumn
            << info.processName
            << std::endl;
    }

    return 0;
}
