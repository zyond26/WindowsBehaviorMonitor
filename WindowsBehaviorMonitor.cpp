// WindowsBehaviorMonitor.cpp : Windows Behavior Monitoring System

#include <iostream>
#include <string>
#include <iomanip>
#include <limits>

#include "ProcessManager.h"

void PrintHeader()
{
    std::wcout << L"\n========================================\n";
    std::wcout << L"  Windows Behavior Monitor System\n";
    std::wcout << L"========================================\n\n";
}

void PrintMenu()
{
    std::wcout << L"Menu:\n";
    std::wcout << L"  1. List all running processes\n";
    std::wcout << L"  2. Scan process memory for suspicious regions\n";
    std::wcout << L"  3. Enable SeDebugPrivilege\n";
    std::wcout << L"  4. Scan all processes for suspicious memory\n";
    std::wcout << L"  0. Exit\n\n";
    std::wcout << L"Select an option: ";
}

void ListAllProcesses(ProcessManager& manager)
{
    std::wcout << L"\n--- Running Processes ---\n\n";
    
    const auto processes = manager.GetRunningProcesses();

    constexpr int kPidWidth = 8;
    constexpr int kParentPidWidth = 12;
    constexpr int kCreationWidth = 24;

    std::wcout << std::left
               << std::setw(kPidWidth) << L"PID"
               << std::setw(kParentPidWidth) << L"ParentPID"
               << std::setw(kCreationWidth) << L"CreationTimeTicks"
               << L"ProcessName" << std::endl;
    std::wcout << std::wstring(80, L'-') << std::endl;

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

    std::wcout << L"\nTotal processes: " << processes.size() << std::endl;
}

void ScanSingleProcess(ProcessManager& manager)
{
    std::wcout << L"\n--- Scan Process Memory ---\n\n";
    std::wcout << L"Enter Process ID (PID): ";
    
    DWORD pid;
    std::wcin >> pid;

    if (std::wcin.fail())
    {
        std::wcin.clear();
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
        std::wcout << L"Invalid PID!\n";
        return;
    }

    std::wcout << L"\nScanning process " << pid << L"...\n\n";
    
    std::wstring warnings = manager.ScanProcessMemory(pid);
    
    if (warnings.empty())
    {
        std::wcout << L"No suspicious memory regions found (or process is allowlisted).\n";
    }
    else
    {
        std::wcout << L"WARNING: Suspicious memory regions detected:\n\n";
        std::wcout << warnings;
    }
}

void EnableDebugPrivilege()
{
    std::wcout << L"\n--- Enable SeDebugPrivilege ---\n\n";
    
    if (ProcessManager::EnableSeDebugPrivilege())
    {
        std::wcout << L"SUCCESS: SeDebugPrivilege enabled!\n";
        std::wcout << L"You can now scan protected processes.\n";
    }
    else
    {
        std::wcout << L"FAILED: Could not enable SeDebugPrivilege.\n";
        std::wcout << L"Make sure you run this program as Administrator.\n";
    }
}

void ScanAllProcesses(ProcessManager& manager)
{
    std::wcout << L"\n--- Scan All Processes ---\n\n";
    std::wcout << L"Scanning all running processes for suspicious memory regions...\n\n";

    const auto processes = manager.GetRunningProcesses();
    int suspiciousCount = 0;
    int scannedCount = 0;

    for (const auto& pair : processes)
    {
        const auto& info = pair.second;
        std::wstring warnings = manager.ScanProcessMemory(info.pid);
        
        if (!warnings.empty())
        {
            suspiciousCount++;
            std::wcout << L"\n[!] Process: " << info.processName 
                       << L" (PID: " << info.pid << L")\n";
            std::wcout << warnings;
        }
        scannedCount++;
    }

    std::wcout << L"\n========================================\n";
    std::wcout << L"Scan Summary:\n";
    std::wcout << L"  Total processes scanned: " << scannedCount << std::endl;
    std::wcout << L"  Suspicious processes found: " << suspiciousCount << std::endl;
    std::wcout << L"========================================\n";
}

int main()
{
    ProcessManager manager;
    bool running = true;
    int choice;

    PrintHeader();
    std::wcout << L"Initializing Windows Behavior Monitor...\n";
    std::wcout << L"Note: Some features require Administrator privileges.\n";

    while (running)
    {
        PrintMenu();
        std::wcin >> choice;

        if (std::wcin.fail())
        {
            std::wcin.clear();
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            std::wcout << L"\nInvalid input! Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            ListAllProcesses(manager);
            break;
        case 2:
            ScanSingleProcess(manager);
            break;
        case 3:
            EnableDebugPrivilege();
            break;
        case 4:
            ScanAllProcesses(manager);
            break;
        case 0:
            std::wcout << L"\nExiting Windows Behavior Monitor. Goodbye!\n";
            running = false;
            break;
        default:
            std::wcout << L"\nInvalid option! Please select 0-4.\n";
            break;
        }

        if (running)
        {
            std::wcout << L"\nPress Enter to continue...";
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            std::wcin.get();
            std::wcout << L"\n\n";
        }
    }

    return 0;
}
