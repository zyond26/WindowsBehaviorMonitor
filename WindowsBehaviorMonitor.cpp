// WindowsBehaviorMonitor.cpp : Integrated Windows Behavior Monitoring System
// Author: Phùng Đức Anh, Nguyễn Thị Diệu, Nguyễn Trí Như

#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>

// Module includes
#include "PMM/ProcessManager.h"
#include "PFM/RegistryMonitor.h"
#include "PFM/StartupMonitor.h"
#include "NMM/NetworkMonitor.h"
#include "Common/Logger.h"
#include "Common/EventStruct.h"

// Global monitoring state
std::atomic<bool> g_pmmRunning{ false };
std::atomic<bool> g_pfmRunning{ false };
std::atomic<bool> g_nmmRunning{ false };

// Module instances
ProcessManager g_processManager;
RegistryMonitor* g_registryMonitor = nullptr;
StartupMonitor* g_startupMonitor = nullptr;

// Terminal color codes for Windows
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void ResetColor() {
    SetColor(7); // Default white
}

void ClearScreen()
{
    system("cls");
}

void PrintBanner()
{
    ClearScreen();
    SetColor(11); // Cyan
    std::wcout << L"\n";
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"  ||                                                                    ||\n";
    std::wcout << L"  ||        W I N D O W S   B E H A V I O R   M O N I T O R            ||\n";
    std::wcout << L"  ||                                                                    ||\n";
    SetColor(14); // Yellow
    std::wcout << L"  ||              BEHAVIOR MONITOR SYSTEM v1.0                          ||\n";
    SetColor(8); // Gray
    std::wcout << L"  ||          User-Mode Endpoint Detection & Response Tool             ||\n";
    ResetColor();
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"\n";
}

void PrintMainMenu()
{
    SetColor(10); // Green
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"  |                         MAIN MENU                                    |\n";
    std::wcout << L"  ========================================================================\n";
    ResetColor();

    std::wcout << L"\n";
    SetColor(14); // Yellow
    std::wcout << L"  [ MODULE SELECTION ]\n";
    ResetColor();
    std::wcout << L"\n";
    std::wcout << L"    [1] PMM - Process & Memory Monitoring\n";
    std::wcout << L"        >> Monitor processes and detect memory injection\n";
    std::wcout << L"\n";
    std::wcout << L"    [2] PFM - Persistence & File-system Monitoring\n";
    std::wcout << L"        >> Monitor Registry & Startup folder changes\n";
    std::wcout << L"\n";
    std::wcout << L"    [3] NMM - Network & System Monitoring Module\n";
    std::wcout << L"        >> Monitor TCP, Files, and Process events\n";
    std::wcout << L"\n";
    SetColor(12); // Red
    std::wcout << L"    [0] Exit Program\n";
    ResetColor();
    std::wcout << L"\n";
    std::wcout << L"  ------------------------------------------------------------------------\n";
    SetColor(11); // Cyan
    std::wcout << L"  >> Select option: ";
    ResetColor();
}

void PrintPMMMenu()
{
    ClearScreen();
    PrintBanner();

    SetColor(13); // Magenta
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"  |         PMM - Process & Memory Monitoring Module                   |\n";
    std::wcout << L"  ========================================================================\n";
    ResetColor();

    std::wcout << L"\n";
    std::wcout << L"    [1] List All Running Processes\n";
    std::wcout << L"    [2] Scan Single Process Memory\n";
    std::wcout << L"    [3] Enable SeDebugPrivilege (Administrator)\n";
    std::wcout << L"    [4] Scan All Processes\n";
    std::wcout << L"    [5] Test Memory Scanner (MockMalwareSim)\n";
    std::wcout << L"    [6] Scan All Processes (Basic Mode)\n";
    std::wcout << L"\n";
    SetColor(12); // Red
    std::wcout << L"    [0] Back to Main Menu\n";
    ResetColor();
    std::wcout << L"\n";
    std::wcout << L"  ------------------------------------------------------------------------\n";
    SetColor(11); // Cyan
    std::wcout << L"  >> Select option: ";
    ResetColor();
}

void PrintPFMMenu()
{
    ClearScreen();
    PrintBanner();

    SetColor(10); // Green
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"  |      PFM - Persistence & File-system Monitoring Module            |\n";
    std::wcout << L"  ========================================================================\n";
    ResetColor();

    std::wcout << L"\n";
    std::wcout << L"    [1] Start Registry & Startup Monitoring (Real-time)\n";
    std::wcout << L"    [2] Show Registry Baseline (HKCU\\Run)\n";
    std::wcout << L"    [3] List Startup Folder Files\n";
    std::wcout << L"\n";
    SetColor(12); // Red
    std::wcout << L"    [0] Back to Main Menu\n";
    ResetColor();
    std::wcout << L"\n";
    std::wcout << L"  ------------------------------------------------------------------------\n";
    SetColor(11); // Cyan
    std::wcout << L"  >> Select option: ";
    ResetColor();
}

void PrintNMMMenu()
{
    ClearScreen();
    PrintBanner();

    SetColor(14); // Yellow
    std::wcout << L"  ========================================================================\n";
    std::wcout << L"  |            NMM - Network & System Monitoring Module                |\n";
    std::wcout << L"  ========================================================================\n";
    ResetColor();

    std::wcout << L"\n";
    std::wcout << L"    [1] Start Network Monitoring (TCP Connections)\n";
    std::wcout << L"    [2] Start File System Monitoring (C:\\TestWatch)\n";
    std::wcout << L"    [3] Start Process Monitoring (New Processes)\n";
    std::wcout << L"    [4] Integrated Monitoring\n";
    ResetColor();
    std::wcout << L"    [5] Stop All Monitoring\n";
    std::wcout << L"    [6] Display Current TCP Connections\n";
    std::wcout << L"\n";
    SetColor(12); // Red
    std::wcout << L"    [0] Back to Main Menu\n";
    ResetColor();
    std::wcout << L"\n  ";
    SetColor(8); // Gray
    std::wcout << L"------------------------------------------------------------------------\n";
    std::wcout << L"  Status: ";

    if (g_pmmRunning) {
        SetColor(10); std::wcout << L"PMM:ON "; ResetColor();
    }
    if (g_pfmRunning) {
        SetColor(10); std::wcout << L"PFM:ON "; ResetColor();
    }
    if (g_nmmRunning) {
        SetColor(10); std::wcout << L"NMM:ON "; ResetColor();
    }
    if (!g_pmmRunning && !g_pfmRunning && !g_nmmRunning) {
        SetColor(8); std::wcout << L"All modules idle"; ResetColor();
    }

    std::wcout << L"\n";
    SetColor(8);
    std::wcout << L"  ------------------------------------------------------------------------\n";
    ResetColor();
}
void PrintStatusBar()
{
    SetColor(8); // Gray
    std::wcout << L"  [Status] ";
    if (g_pmmRunning) {
        SetColor(10); std::wcout << L"PMM:ON "; ResetColor();
    }
    if (g_pfmRunning) {
        SetColor(10); std::wcout << L"PFM:ON "; ResetColor();
    }
    if (g_nmmRunning) {
        SetColor(10); std::wcout << L"NMM:ON "; ResetColor();
    }
    if (!g_pmmRunning && !g_pfmRunning && !g_nmmRunning) {
        SetColor(8); std::wcout << L"All modules idle"; ResetColor();
    }
    ResetColor();
    std::wcout << L"\n";
}

void WaitForEnter()
{
    std::wcout << L"\n";
    SetColor(8);
    std::wcout << L"  Press Enter to continue...";
    ResetColor();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
    std::wcin.get();
}

// Helper function to write suspicious findings to separate log file
void LogSuspiciousFindings(const std::wstring& processName, DWORD pid, const std::wstring& details)
{
    std::wofstream logFile("suspicious_findings.log", std::ios::app);
    if (logFile.is_open())
    {
        SYSTEMTIME st;
        GetLocalTime(&st);

        wchar_t timeStr[64];
        swprintf_s(timeStr, L"[%02d:%02d:%02d]", st.wHour, st.wMinute, st.wSecond);

        logFile << L"\n========================================\n";
        logFile << timeStr << L" SUSPICIOUS PROCESS DETECTED\n";
        logFile << L"========================================\n";
        logFile << L"Process: " << processName << L"\n";
        logFile << L"PID: " << pid << L"\n";
        logFile << L"\nMemory Analysis:\n";
        logFile << details;
        logFile << L"========================================\n";
        logFile.flush();
        logFile.close();
    }
}

// Helper function to log NMM events
void LogNMMEvent(const std::string& eventType, const std::string& detail)
{
    std::ofstream logFile("nmm_events.log", std::ios::app);
    if (logFile.is_open())
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        char timeStr[64];
        sprintf_s(timeStr, "[%04d-%02d-%02d %02d:%02d:%02d]", 
                  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        
        logFile << timeStr << " [" << eventType << "] " << detail << "\n";
        logFile.flush();
        logFile.close();
    }
}

// ========== PMM Module Functions ==========

void ListAllProcesses()
{
    ClearScreen();
    PrintBanner();

    std::wcout << L"\n";
    SetColor(14);
    std::wcout << L"  +----- Running Processes ---------------------------------------------+\n";
    ResetColor();
    std::wcout << L"\n";

    const auto processes = g_processManager.GetRunningProcesses();

    constexpr int kPidWidth = 8;
    constexpr int kParentPidWidth = 12;
    constexpr int kCreationWidth = 24;

    std::wcout << L"  " << std::left
        << std::setw(kPidWidth) << L"PID"
        << std::setw(kParentPidWidth) << L"ParentPID"
        << std::setw(kCreationWidth) << L"CreationTimeTicks"
        << L"ProcessName" << std::endl;
    std::wcout << L"  " << std::wstring(75, L'-') << std::endl;

    for (const auto& pair : processes)
    {
        const auto& info = pair.second;
        const std::wstring creationColumn = info.creationTime == 0
            ? L"undefined"
            : std::to_wstring(info.creationTime);
        std::wcout << L"  " << std::left
            << std::setw(kPidWidth) << info.pid
            << std::setw(kParentPidWidth) << info.parentPid
            << std::setw(kCreationWidth) << creationColumn
            << info.processName
            << std::endl;
    }

    std::wcout << L"\n";
    SetColor(10);
    std::wcout << L"  Total processes: " << processes.size() << std::endl;
    ResetColor();
}

void ScanSingleProcess()
{
    ClearScreen();
    PrintBanner();

    std::wcout << L"\n";
    SetColor(14);
    std::wcout << L"  +----- Scan Process Memory -------------------------------------------+\n";
    ResetColor();
    std::wcout << L"\n  Enter Process ID (PID): ";

    DWORD pid;
    std::wcin >> pid;

    if (std::wcin.fail())
    {
        std::wcin.clear();
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
        SetColor(12);
        std::wcout << L"\n  [X] Invalid PID!\n";
        ResetColor();
        return;
    }

    std::wcout << L"\n  Scanning process " << pid << L"...\n\n";

    std::wstring warnings = g_processManager.ScanProcessMemory(pid);

    if (warnings.empty())
    {
        SetColor(10);
        std::wcout << L"  [OK] No suspicious memory regions found.\n";
        ResetColor();
    }
    else
    {
        SetColor(12);
        std::wcout << L"  [!] WARNING: Suspicious memory regions detected:\n\n";
        ResetColor();
        std::wcout << L"  " << warnings;
    }
}

void EnableDebugPrivilege()
{
    ClearScreen();
    PrintBanner();

    std::wcout << L"\n";
    SetColor(14);
    std::wcout << L"  +----- Enable SeDebugPrivilege ---------------------------------------+\n";
    ResetColor();
    std::wcout << L"\n";

    if (ProcessManager::EnableSeDebugPrivilege())
    {
        SetColor(10);
        std::wcout << L"  [OK] SUCCESS: SeDebugPrivilege enabled!\n";
        std::wcout << L"  You can now scan protected processes.\n";
        ResetColor();
    }
    else
    {
        SetColor(12);
        std::wcout << L"  [X] FAILED: Could not enable SeDebugPrivilege.\n";
        std::wcout << L"  Make sure you run this program as Administrator.\n";
        ResetColor();
    }
}

void TestMemoryScanner()
{
    ClearScreen();
    PrintBanner();

    std::wcout << L"\n";
    SetColor(14);
    std::wcout << L"  +----- Test Memory Scanner -------------------------------------------+\n";
    ResetColor();
    std::wcout << L"\n  Instructions:\n";
    std::wcout << L"    1. Run MockMalwareSim.exe in another terminal\n";
    std::wcout << L"    2. Note the PID displayed by MockMalwareSim\n";
    std::wcout << L"    3. Enter that PID below\n\n";

    std::wcout << L"  Enter MockMalwareSim PID: ";

    DWORD pid;
    std::wcin >> pid;

    if (std::wcin.fail())
    {
        std::wcin.clear();
        std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
        SetColor(12);
        std::wcout << L"\n  [X] Invalid PID!\n";
        ResetColor();
        return;
    }

    g_processManager.TestMemoryScanner(pid);
}

void HandlePMMModule()
{
    bool inPMM = true;
    int choice;

    while (inPMM)
    {
        PrintPMMMenu();
        std::wcin >> choice;

        if (std::wcin.fail())
        {
            std::wcin.clear();
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            continue;
        }

        switch (choice)
        {
        case 1:
            ListAllProcesses();
            WaitForEnter();
            break;
        case 2:
            ScanSingleProcess();
            WaitForEnter();
            break;
        case 3:
            EnableDebugPrivilege();
            WaitForEnter();
            break;
        case 4:
        {
            ClearScreen();
            PrintBanner();
            std::wcout << L"\n";
            SetColor(14);
            std::wcout << L"  +----- Continuous Process Monitoring (Enhanced) ----------------------+\n";
            ResetColor();
            std::wcout << L"\n";

            // Clear old log file
            std::wofstream clearLog("suspicious_findings.log", std::ios::trunc);
            if (clearLog.is_open())
            {
                clearLog << L"===========================================\n";
                clearLog << L"  SUSPICIOUS PROCESS FINDINGS LOG\n";
                clearLog << L"===========================================\n";
                clearLog.close();
            }

            SetColor(10);
            std::wcout << L"  [*] Opening separate terminal for suspicious findings...\n";
            ResetColor();

            // Start PowerShell to tail the log file
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            wchar_t cmd[] = L"powershell.exe -NoExit -Command \"Write-Host 'SUSPICIOUS FINDINGS MONITOR' -ForegroundColor Red; Get-Content suspicious_findings.log -Wait\"";

            if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            SetColor(10);
            std::wcout << L"  [OK] Starting continuous process monitoring...\n";
            SetColor(14);
            std::wcout << L"\n  Press 'Q' to stop monitoring...\n\n";
            ResetColor();

            std::atomic<bool> monitorRunning(true);
            std::thread monitorThread([&monitorRunning]() {
                auto previousProcesses = g_processManager.GetRunningProcesses();

                while (monitorRunning.load())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    auto currentProcesses = g_processManager.GetRunningProcesses();

                    for (const auto& pair : currentProcesses)
                    {
                        if (!monitorRunning.load()) break;

                        DWORD pid = pair.first;
                        if (previousProcesses.find(pid) == previousProcesses.end())
                        {
                            SetColor(10);
                            std::wcout << L"  [+] NEW: ";
                            ResetColor();
                            std::wcout << pair.second.processName << L" (PID: " << pid << L")\n";

                            std::wstring scanResult = g_processManager.ScanProcessMemory(pid);
                            if (!scanResult.empty())
                            {
                                SetColor(12);
                                std::wcout << L"      [!!!] SUSPICIOUS MEMORY DETECTED!\n";
                                ResetColor();
                                LogSuspiciousFindings(pair.second.processName, pid, scanResult);
                            }
                        }
                    }

                    for (const auto& pair : previousProcesses)
                    {
                        if (!monitorRunning.load()) break;

                        DWORD pid = pair.first;
                        if (currentProcesses.find(pid) == currentProcesses.end())
                        {
                            SetColor(8);
                            std::wcout << L"  [-] TERMINATED: ";
                            ResetColor();
                            std::wcout << pair.second.processName << L" (PID: " << pid << L")\n";
                        }
                    }

                    previousProcesses = currentProcesses;
                }
                });

            while (monitorRunning.load())
            {
                if (_kbhit())
                {
                    int key = _getch();
                    if (key == 'q' || key == 'Q')
                    {
                        monitorRunning.store(false);
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            monitorThread.join();

            SetColor(10);
            std::wcout << L"\n  [OK] Monitoring stopped.\n";
            ResetColor();
            WaitForEnter();
        }
        break;
        case 5:
            TestMemoryScanner();
            WaitForEnter();
            break;
        case 6:
        {
            ClearScreen();
            PrintBanner();
            std::wcout << L"\n";
            SetColor(14);
            std::wcout << L"  +----- Continuous Process Monitoring ---------------------------------+\n";
            ResetColor();
            std::wcout << L"\n";
            SetColor(10);
            std::wcout << L"  [OK] Starting continuous process monitoring...\n";
            SetColor(14);
            std::wcout << L"  Press 'Q' to stop monitoring...\n\n";
            ResetColor();

            std::atomic<bool> monitorRunning(true);
            std::thread monitorThread([&monitorRunning]() {
                g_processManager.StartContinuousMonitoring(monitorRunning);
                });

            while (monitorRunning.load())
            {
                if (_kbhit())
                {
                    int key = _getch();
                    if (key == 'q' || key == 'Q')
                    {
                        monitorRunning.store(false);
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            monitorThread.join();

            SetColor(10);
            std::wcout << L"\n  [OK] Monitoring stopped.\n";
            ResetColor();
            WaitForEnter();
        }
        break;
        case 0:
            inPMM = false;
            break;
        default:
            ClearScreen();
            PrintBanner();
            SetColor(12);
            std::wcout << L"\n  [X] Invalid option!\n";
            ResetColor();
            WaitForEnter();
            break;
        }
    }
}

// ========== PFM Module Functions ==========

std::thread* g_regMonitorThread = nullptr;
std::thread* g_startupMonitorThread = nullptr;

void HandlePFMModule()
{
    bool inPFM = true;
    int choice;

    while (inPFM)
    {
        PrintPFMMenu();
        std::wcin >> choice;

        if (std::wcin.fail())
        {
            std::wcin.clear();
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            ClearScreen();
            PrintBanner();
            if (g_pfmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
                WaitForEnter();
            }
            else
            {
                g_pfmRunning = true;

                if (!g_registryMonitor) g_registryMonitor = new RegistryMonitor();
                if (!g_startupMonitor) g_startupMonitor = new StartupMonitor();

                g_regMonitorThread = new std::thread([&]() {
                    g_registryMonitor->Start();
                    });

                g_startupMonitorThread = new std::thread([&]() {
                    g_startupMonitor->Start();
                    });

                SetColor(10);
                std::wcout << L"\n  [OK] PFM Monitoring started!\n";
                std::wcout << L"  Monitoring:\n";
                std::wcout << L"    - Registry: HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\n";
                std::wcout << L"    - Startup Folder\n";
                SetColor(14);
                std::wcout << L"\n  Press any key to stop monitoring...\n\n";
                ResetColor();

                while (g_pfmRunning)
                {
                    if (_kbhit())
                    {
                        _getch();
                        g_pfmRunning = false;
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }

                if (g_registryMonitor) g_registryMonitor->Stop();
                if (g_startupMonitor) g_startupMonitor->Stop();

                if (g_regMonitorThread)
                {
                    g_regMonitorThread->join();
                    delete g_regMonitorThread;
                    g_regMonitorThread = nullptr;
                }
                if (g_startupMonitorThread)
                {
                    g_startupMonitorThread->join();
                    delete g_startupMonitorThread;
                    g_startupMonitorThread = nullptr;
                }

                SetColor(10);
                std::wcout << L"\n  [OK] PFM Monitoring stopped.\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            }
        }
        break;

        case 2:
        {
            ClearScreen();
            PrintBanner();
            std::wcout << L"\n";
            SetColor(14);
            std::wcout << L"  +----- Registry Baseline ---------------------------------------------+\n";
            ResetColor();
            std::wcout << L"\n";
            if (!g_registryMonitor) g_registryMonitor = new RegistryMonitor();
            g_registryMonitor->PrintBaseline();
            std::wcout << L"\n";
            WaitForEnter();
        }
        break;

        case 3:
        {
            ClearScreen();
            PrintBanner();
            std::wcout << L"\n";
            SetColor(14);
            std::wcout << L"  +----- Startup Folder Files ------------------------------------------+\n";
            ResetColor();
            std::wcout << L"\n";
            if (!g_startupMonitor) g_startupMonitor = new StartupMonitor();
            auto files = g_startupMonitor->ListStartupFiles();
            if (files.empty())
            {
                std::wcout << L"  (No files found)\n";
            }
            else
            {
                for (const auto& file : files)
                {
                    std::wcout << L"    - " << file << L"\n";
                }
            }
            std::wcout << L"\n";
            WaitForEnter();
        }
        break;

        case 0:
            if (g_pfmRunning)
            {
                g_pfmRunning = false;
                if (g_registryMonitor) g_registryMonitor->Stop();
                if (g_startupMonitor) g_startupMonitor->Stop();
                if (g_regMonitorThread && g_regMonitorThread->joinable())
                {
                    g_regMonitorThread->join();
                    delete g_regMonitorThread;
                    g_regMonitorThread = nullptr;
                }
                if (g_startupMonitorThread && g_startupMonitorThread->joinable())
                {
                    g_startupMonitorThread->join();
                    delete g_startupMonitorThread;
                    g_startupMonitorThread = nullptr;
                }
            }
            inPFM = false;
            break;

        default:
            ClearScreen();
            PrintBanner();
            SetColor(12);
            std::wcout << L"\n  [X] Invalid option!\n";
            ResetColor();
            WaitForEnter();
            break;
        }
    }
}

// ========== NMM Module Functions ==========

std::thread* g_nmmNetworkThread = nullptr;
std::thread* g_nmmFileThread = nullptr;
std::thread* g_nmmProcessThread = nullptr;

void NetworkMonitoringLoop()
{
    std::wcout << L"\n";
    SetColor(10);
    std::wcout << L"  [OK] Network monitoring started...\n";
    SetColor(14);
    std::wcout << L"  Press any key to stop monitoring...\n\n";
    ResetColor();

    while (g_nmmRunning)
    {
        if (_kbhit())
        {
            _getch();
            g_nmmRunning = false;
            break;
        }

        SysEvent event;
        if (MonitorTCPConnections(event))
        {
            SetColor(14);
            std::wcout << L"  [NMM-NET] ";
            SetColor(11);
            std::wcout << L"New Connection: ";
            ResetColor();

            std::wstring wDetail(event.detail.begin(), event.detail.end());
            std::wcout << wDetail << L"\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    SetColor(10);
    std::wcout << L"\n  [OK] Network monitoring stopped.\n";
    ResetColor();
}

void FileMonitoringLoop()
{
    std::wcout << L"\n";
    SetColor(10);
    std::wcout << L"  [OK] File system monitoring started...\n";
    SetColor(14);
    std::wcout << L"  Press any key to stop monitoring...\n\n";
    ResetColor();

    while (g_nmmRunning)
    {
        if (_kbhit())
        {
            _getch();
            g_nmmRunning = false;
            break;
        }

        SysEvent event;
        if (MonitorDirectory(event))
        {
            SetColor(14);
            std::wcout << L"  [NMM-FILE] ";
            SetColor(11);
            std::wcout << L"File Change: ";
            ResetColor();

            std::wstring wDetail(event.detail.begin(), event.detail.end());
            std::wcout << wDetail << L"\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    SetColor(10);
    std::wcout << L"\n  [OK] File monitoring stopped.\n";
    ResetColor();
}

void ProcessMonitoringLoop()
{
    std::wcout << L"\n";
    SetColor(10);
    std::wcout << L"  [OK] Process monitoring started...\n";
    SetColor(14);
    std::wcout << L"  Press any key to stop monitoring...\n\n";
    ResetColor();

    while (g_nmmRunning)
    {
        if (_kbhit())
        {
            _getch();
            g_nmmRunning = false;
            break;
        }

        SysEvent event;
        if (CheckNewProcess(event))
        {
            SetColor(14);
            std::wcout << L"  [NMM-PROC] ";
            SetColor(11);
            std::wcout << L"New Process: ";
            ResetColor();

            std::wstring wDetail(event.detail.begin(), event.detail.end());
            std::wcout << wDetail << L"\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    SetColor(10);
    std::wcout << L"\n  [OK] Process monitoring stopped.\n";
    ResetColor();
}

void HandleNMMModule()
{
    bool inNMM = true;
    int choice;

    while (inNMM)
    {
        PrintNMMMenu();
        std::wcin >> choice;

        if (std::wcin.fail())
        {
            std::wcin.clear();
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            ClearScreen();
            PrintBanner();
            if (g_nmmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
                WaitForEnter();
            }
            else
            {
                g_nmmRunning = true;
                g_nmmNetworkThread = new std::thread(NetworkMonitoringLoop);
                g_nmmNetworkThread->join();
                delete g_nmmNetworkThread;
                g_nmmNetworkThread = nullptr;

                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        break;

        case 2:
        {
            ClearScreen();
            PrintBanner();
            if (g_nmmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
                WaitForEnter();
            }
            else
            {
                g_nmmRunning = true;
                g_nmmFileThread = new std::thread(FileMonitoringLoop);
                g_nmmFileThread->join();
                delete g_nmmFileThread;
                g_nmmFileThread = nullptr;

                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        break;

        case 3:
        {
            ClearScreen();
            PrintBanner();
            if (g_nmmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
                WaitForEnter();
            }
            else
            {
                g_nmmRunning = true;
                g_nmmProcessThread = new std::thread(ProcessMonitoringLoop);
                g_nmmProcessThread->join();
                delete g_nmmProcessThread;
                g_nmmProcessThread = nullptr;

                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
        break;

        case 4:
        {
            ClearScreen();
            PrintBanner();
            if (g_nmmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
                WaitForEnter();
            }
            else
            {
                std::wcout << L"\n";
                SetColor(14);
                std::wcout << L"  +----- Integrated NMM Monitoring (Enhanced) --------------------------+\n";
                ResetColor();
                std::wcout << L"\n";
                
                // Clear old log file
                {
                    std::ofstream clearLog("nmm_events.log", std::ios::trunc);
                    if (clearLog.is_open())
                    {
                        clearLog << "===========================================\n";
                        clearLog << "  NMM EVENT LOG (Network, File, Process)\n";
                        clearLog << "  Started: ";
                        SYSTEMTIME st;
                        GetLocalTime(&st);
                        char timeStr[64];
                        sprintf_s(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", 
                                  st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                        clearLog << timeStr;
                        clearLog << "\n===========================================\n";
                        clearLog.close();
                    }
                }
                
                // Open separate terminal for detailed log
                SetColor(10);
                std::wcout << L"  [*] Opening separate terminal for event details...\n";
                ResetColor();
                
                STARTUPINFO si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                std::wstring cmd = L"powershell.exe -NoExit -Command \"Write-Host 'NMM EVENT MONITOR' -ForegroundColor Cyan; Write-Host '========================================' -ForegroundColor Yellow; Get-Content nmm_events.log -Wait\"";
                
                if (CreateProcess(NULL, &cmd[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                SetColor(10);
                std::wcout << L"  [OK] Starting integrated monitoring...\n";
                std::wcout << L"  [*] Main terminal: Event summaries\n";
                std::wcout << L"  [*] Second terminal: Full event details\n";
                SetColor(14);
                std::wcout << L"\n  Press 'Q' to stop monitoring...\n\n";
                ResetColor();
                
                g_nmmRunning = true;
                
                std::thread monitorThread([&]() {
                    while (g_nmmRunning)
                    {
                        // Network monitoring
                        {
                            SysEvent event;
                            if (MonitorTCPConnections(event))
                            {
                                SetColor(11);
                                std::wcout << L"  [NET] ";
                                ResetColor();
                                std::wstring wDetail(event.detail.begin(), event.detail.end());
                                std::wcout << wDetail << L"\n";
                                
                                // Log to file
                                LogNMMEvent("Network", event.detail);
                            }
                        }
                        
                        // File monitoring
                        {
                            SysEvent event;
                            if (MonitorDirectory(event))
                            {
                                SetColor(14);
                                std::wcout << L"  [FILE] ";
                                ResetColor();
                                std::wstring wDetail(event.detail.begin(), event.detail.end());
                                std::wcout << wDetail << L"\n";
                                
                                // Log to file
                                LogNMMEvent("File", event.detail);
                            }
                        }
                        
                        // Process monitoring
                        {
                            SysEvent event;
                            if (CheckNewProcess(event))
                            {
                                SetColor(10);
                                std::wcout << L"  [PROC] ";
                                ResetColor();
                                std::wstring wDetail(event.detail.begin(), event.detail.end());
                                std::wcout << wDetail << L"\n";
                                
                                // Log to file
                                LogNMMEvent("Process", event.detail);
                            }
                        }
                        
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                });
                
                // Wait for user to press 'Q'
                while (g_nmmRunning)
                {
                    if (_kbhit())
                    {
                        int key = _getch();
                        if (key == 'q' || key == 'Q')
                        {
                            g_nmmRunning = false;
                            break;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                
                monitorThread.join();
                
                SetColor(10);
                std::wcout << L"\n  [OK] Integrated monitoring stopped.\n";
                SetColor(8);
                std::wcout << L"  Note: Check 'nmm_events.log' for full event history.\n";
                ResetColor();
                WaitForEnter();
            }
        }
        break;

        case 5:
        {
            ClearScreen();
            PrintBanner();
            if (!g_nmmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] No monitoring is running!\n";
                ResetColor();
            }
            else
            {
                g_nmmRunning = false;

                if (g_nmmNetworkThread) { g_nmmNetworkThread->join(); delete g_nmmNetworkThread; g_nmmNetworkThread = nullptr; }
                if (g_nmmFileThread) { g_nmmFileThread->join(); delete g_nmmFileThread; g_nmmFileThread = nullptr; }
                if (g_nmmProcessThread) { g_nmmProcessThread->join(); delete g_nmmProcessThread; g_nmmProcessThread = nullptr; }

                SetColor(10);
                std::wcout << L"\n  [OK] All monitoring stopped!\n";
                ResetColor();
            }
            WaitForEnter();
        }
        break;

        case 6:
        {
            ClearScreen();
            PrintBanner();
            std::wcout << L"\n";
            SetColor(14);
            std::wcout << L"  +----- Current TCP Connections ---------------------------------------+\n";
            ResetColor();
            std::wcout << L"\n";
            std::wcout << L"  Scanning TCP table...\n\n";

            for (int i = 0; i < 10; i++)
            {
                SysEvent event;
                if (MonitorTCPConnections(event))
                {
                    std::wstring wDetail(event.detail.begin(), event.detail.end());
                    std::wcout << L"    - " << wDetail << L"\n";
                }
            }

            std::wcout << L"\n";
            WaitForEnter();
        }
        break;

        case 0:
            if (g_nmmRunning)
            {
                g_nmmRunning = false;
                if (g_nmmNetworkThread) { g_nmmNetworkThread->join(); delete g_nmmNetworkThread; g_nmmNetworkThread = nullptr; }
                if (g_nmmFileThread) { g_nmmFileThread->join(); delete g_nmmFileThread; g_nmmFileThread = nullptr; }
                if (g_nmmProcessThread) { g_nmmProcessThread->join(); delete g_nmmProcessThread; g_nmmProcessThread = nullptr; }
            }
            inNMM = false;
            break;

        default:
            ClearScreen();
            PrintBanner();
            SetColor(12);
            std::wcout << L"\n  [X] Invalid option!\n";
            ResetColor();
            WaitForEnter();
            break;
        }
    }
}

// ========== Main Program ==========

int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    CONSOLE_FONT_INFOEX cfi = {};
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);

    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    Logger::Instance().SetLogFile(L"WinBehaviorMonitor.log");

    bool running = true;
    int choice;

    PrintBanner();

    SetColor(14);
    std::wcout << L"  System Initialization...\n";
    ResetColor();
    std::wcout << L"  ---------------------------------------------------------------------\n";
    std::wcout << L"    - PMM Module: Ready\n";
    std::wcout << L"    - PFM Module: Ready\n";
    std::wcout << L"    - NMM Module: Ready\n";
    std::wcout << L"  ---------------------------------------------------------------------\n";
    SetColor(8);
    std::wcout << L"\n  Note: Some features require Administrator privileges.\n";
    ResetColor();

    WaitForEnter();

    while (running)
    {
        ClearScreen();
        PrintBanner();
        PrintStatusBar();
        PrintMainMenu();

        std::wcin >> choice;

        if (std::wcin.fail())
        {
            std::wcin.clear();
            std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
            continue;
        }

        switch (choice)
        {
        case 1:
            HandlePMMModule();
            break;
        case 2:
            HandlePFMModule();
            break;
        case 3:
            HandleNMMModule();
            break;
        case 0:
            if (g_pfmRunning)
            {
                g_pfmRunning = false;
                if (g_registryMonitor) g_registryMonitor->Stop();
                if (g_startupMonitor) g_startupMonitor->Stop();
                if (g_regMonitorThread) { g_regMonitorThread->join(); delete g_regMonitorThread; }
                if (g_startupMonitorThread) { g_startupMonitorThread->join(); delete g_startupMonitorThread; }
            }

            if (g_nmmRunning)
            {
                g_nmmRunning = false;
                if (g_nmmNetworkThread) { g_nmmNetworkThread->join(); delete g_nmmNetworkThread; }
                if (g_nmmFileThread) { g_nmmFileThread->join(); delete g_nmmFileThread; }
                if (g_nmmProcessThread) { g_nmmProcessThread->join(); delete g_nmmProcessThread; }
            }

            delete g_registryMonitor;
            delete g_startupMonitor;

            ClearScreen();
            PrintBanner();
            SetColor(10);
            std::wcout << L"\n  Thank you for using Windows Behavior Monitor!\n";
            std::wcout << L"  Goodbye!\n\n";
            ResetColor();
            running = false;
            break;
        default:
            SetColor(12);
            std::wcout << L"\n  [X] Invalid option! Please select 0-3.\n";
            ResetColor();
            WaitForEnter();
            break;
        }
    }

    return 0;
}
