// WindowsBehaviorMonitor.cpp : Integrated Windows Behavior Monitoring System
// Author: Phùng Đức Anh, Nguyễn Thị Diệu, Nguyễn Trí Như

#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <thread>
#include <atomic>
#include <chrono>
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
    std::wcout << L"    [4] Scan All Processes for Suspicious Memory\n";
    std::wcout << L"    [5] Test Memory Scanner (MockMalwareSim)\n";
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
    std::wcout << L"    [4] Start All Monitoring (Network + File + Process)\n";
    std::wcout << L"    [5] Stop All Monitoring\n";
    std::wcout << L"    [6] Display Current TCP Connections\n";
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

void PrintStatusBar()
{
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

void WaitForEnter()
{
    std::wcout << L"\n";
    SetColor(8);
    std::wcout << L"  Press Enter to continue...";
    ResetColor();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
    std::wcin.get();
}

// ========== PMM Module Functions ==========

void ListAllProcesses()
{
    ClearScreen();
    PrintBanner();
    
    std::wcout << L"\n";
    SetColor(14); // Yellow
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

void ScanAllProcesses()
{
    ClearScreen();
    PrintBanner();
    
    std::wcout << L"\n";
    SetColor(14);
    std::wcout << L"  +----- Scan All Processes --------------------------------------------+\n";
    ResetColor();
    std::wcout << L"\n  Scanning all running processes...\n\n";

    const auto processes = g_processManager.GetRunningProcesses();
    int suspiciousCount = 0;
    int scannedCount = 0;

    for (const auto& pair : processes)
    {
        const auto& info = pair.second;
        std::wstring warnings = g_processManager.ScanProcessMemory(info.pid);
        
        if (!warnings.empty())
        {
            suspiciousCount++;
            SetColor(12);
            std::wcout << L"\n  [!] Process: " << info.processName 
                       << L" (PID: " << info.pid << L")\n";
            ResetColor();
            std::wcout << L"  " << warnings;
        }
        scannedCount++;
    }

    std::wcout << L"\n";
    std::wcout << L"  ===================================================================\n";
    SetColor(14);
    std::wcout << L"  Scan Summary:\n";
    ResetColor();
    std::wcout << L"    - Total processes scanned: " << scannedCount << std::endl;
    if (suspiciousCount > 0) {
        SetColor(12);
        std::wcout << L"    - Suspicious processes found: " << suspiciousCount << std::endl;
        ResetColor();
    } else {
        SetColor(10);
        std::wcout << L"    - No suspicious processes found" << std::endl;
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
            ScanAllProcesses();
            WaitForEnter();
            break;
        case 5:
            TestMemoryScanner();
            WaitForEnter();
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
            ClearScreen();
            PrintBanner();
            if (g_pfmRunning)
            {
                SetColor(14);
                std::wcout << L"\n  [!] Monitoring is already running!\n";
                ResetColor();
            }
            else
            {
                g_pfmRunning = true;
                
                // Initialize monitors
                if (!g_registryMonitor) g_registryMonitor = new RegistryMonitor();
                if (!g_startupMonitor) g_startupMonitor = new StartupMonitor();
                
                // Start in separate threads
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
                std::wcout << L"    - Startup Folder: %APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\n";
                std::wcout << L"\n  Check console output for alerts...\n";
                ResetColor();
            }
            WaitForEnter();
            break;
            
        case 2:
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
            }
            WaitForEnter();
            break;
        //    
        //case 0:
        //    // Stop monitoring before exiting
        //    if (g_pfmRunning)
        //    {
        //        g_pfmRunning = false;
        //        if (g_registryMonitor) g_registryMonitor->Stop();
        //        if (g_startupMonitor) g_startupMonitor->Stop();
        //        if (g_regMonitorThread) {
        //            g_regMonitorThread->join();
        //            delete g_regMonitorThread;
        //            g_regMonitorThread = nullptr;
        //        }
        //        if (g_startupMonitorThread) {
        //            g_startupMonitorThread->join();
        //            delete g_startupMonitorThread;
        //            g_startupMonitorThread = nullptr;
        //        }
        //    }
        //    inPFM = false;
        //    break;

        case 0:
        {
            if (g_pfmRunning)
            {
                g_pfmRunning = false;

                if (g_registryMonitor)
                    g_registryMonitor->Stop();

                if (g_startupMonitor)
                    g_startupMonitor->Stop();

                if (g_regMonitorThread)
                {
                    if (g_regMonitorThread->joinable())
                        g_regMonitorThread->join();

                    delete g_regMonitorThread;
                    g_regMonitorThread = nullptr;
                }

                if (g_startupMonitorThread)
                {
                    if (g_startupMonitorThread->joinable())
                        g_startupMonitorThread->join();

                    delete g_startupMonitorThread;
                    g_startupMonitorThread = nullptr;
                }
            }

            inPFM = false;
            break;
        }
            
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
        // Check for keyboard input
        if (_kbhit())
        {
            _getch(); // Clear the key press
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
            
            // Convert string to wstring for display
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
        // Check for keyboard input
        if (_kbhit())
        {
            _getch(); // Clear the key press
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
        // Check for keyboard input
        if (_kbhit())
        {
            _getch(); // Clear the key press
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
        case 1: // Start Network Monitoring
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
                
                // Wait for thread to finish (when user presses key)
                g_nmmNetworkThread->join();
                delete g_nmmNetworkThread;
                g_nmmNetworkThread = nullptr;
                
                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
            
        case 2: // Start File Monitoring
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
                
                // Wait for thread to finish (when user presses key)
                g_nmmFileThread->join();
                delete g_nmmFileThread;
                g_nmmFileThread = nullptr;
                
                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
            
        case 3: // Start Process Monitoring
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
                
                // Wait for thread to finish (when user presses key)
                g_nmmProcessThread->join();
                delete g_nmmProcessThread;
                g_nmmProcessThread = nullptr;
                
                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
            
        case 4: // Start All Monitoring
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
                g_nmmFileThread = new std::thread(FileMonitoringLoop);
                g_nmmProcessThread = new std::thread(ProcessMonitoringLoop);
                SetColor(10);
                std::wcout << L"\n  [OK] All NMM monitors started!\n";
                std::wcout << L"  - Network: Monitoring TCP connections\n";
                std::wcout << L"  - File: Monitoring C:\\TestWatch\n";
                std::wcout << L"  - Process: Monitoring new processes\n";
                SetColor(14);
                std::wcout << L"\n  Press any key to stop all monitoring...\n";
                ResetColor();
                
                // Wait for threads to finish (when user presses key)
                if (g_nmmNetworkThread) 
                {
                    g_nmmNetworkThread->join();
                    delete g_nmmNetworkThread;
                    g_nmmNetworkThread = nullptr;
                }
                if (g_nmmFileThread) 
                {
                    g_nmmFileThread->join();
                    delete g_nmmFileThread;
                    g_nmmFileThread = nullptr;
                }
                if (g_nmmProcessThread) 
                {
                    g_nmmProcessThread->join();
                    delete g_nmmProcessThread;
                    g_nmmProcessThread = nullptr;
                }
                
                SetColor(8);
                std::wcout << L"\n  Returning to menu...\n";
                ResetColor();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
            break;
            
        case 5: // Stop All Monitoring
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
                
                if (g_nmmNetworkThread)
                {
                    g_nmmNetworkThread->join();
                    delete g_nmmNetworkThread;
                    g_nmmNetworkThread = nullptr;
                }
                
                if (g_nmmFileThread)
                {
                    g_nmmFileThread->join();
                    delete g_nmmFileThread;
                    g_nmmFileThread = nullptr;
                }
                
                if (g_nmmProcessThread)
                {
                    g_nmmProcessThread->join();
                    delete g_nmmProcessThread;
                    g_nmmProcessThread = nullptr;
                }
                
                SetColor(10);
                std::wcout << L"\n  [OK] All monitoring stopped!\n";
                ResetColor();
            }
            WaitForEnter();
            break;
            
        case 6: // Display Current TCP Connections
            {
                ClearScreen();
                PrintBanner();
                std::wcout << L"\n";
                SetColor(14);
                std::wcout << L"  +----- Current TCP Connections ---------------------------------------+\n";
                ResetColor();
                std::wcout << L"\n";
                std::wcout << L"  Scanning TCP table...\n\n";
                
                // Use MonitorTCPConnections to show existing connections
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
            }
            WaitForEnter();
            break;
            
        case 0: // Back to Main Menu
            // Stop monitoring before exiting
            if (g_nmmRunning)
            {
                g_nmmRunning = false;
                
                if (g_nmmNetworkThread) {
                    g_nmmNetworkThread->join();
                    delete g_nmmNetworkThread;
                    g_nmmNetworkThread = nullptr;
                }
                if (g_nmmFileThread) {
                    g_nmmFileThread->join();
                    delete g_nmmFileThread;
                    g_nmmFileThread = nullptr;
                }
                if (g_nmmProcessThread) {
                    g_nmmProcessThread->join();
                    delete g_nmmProcessThread;
                    g_nmmProcessThread = nullptr;
                }
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
    // Enable Virtual Terminal Processing for better Unicode support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // Set console font to one that supports Unicode (Consolas)
    CONSOLE_FONT_INFOEX cfi = {};
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas");
    SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
    
    // Set console to Unicode mode for wcout (UTF-16)
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    
    // Initialize logger
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

