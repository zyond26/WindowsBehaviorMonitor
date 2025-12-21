#include "ProcessManager.h"

#include <TlHelp32.h>
#include <memory>
#include <utility>
#include <iostream>
#include <thread>
#include <chrono>

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

void ProcessManager::TestMemoryScanner(DWORD targetPID)
{
    std::wcout << L"\n========================================\n";
    std::wcout << L"  Memory Scanner Test\n";
    std::wcout << L"========================================\n\n";
    
    std::wcout << L"Target PID: " << targetPID << std::endl;
    
    // Get process info if available
    const auto cached = processes_.find(targetPID);
    if (cached != processes_.end())
    {
        std::wcout << L"Process Name: " << cached->second.processName << std::endl;
    }
    
    std::wcout << L"\nScanning process memory...\n\n";
    
    // Call ScanProcessMemory
    std::wstring detectionResult = ScanProcessMemory(targetPID);
    
    // Analyze results
    if (detectionResult.empty())
    {
        std::wcout << L"TEST FAILED: No RWX regions detected!\n";
        std::wcout << L"Possible reasons:\n";
        std::wcout << L"  1. Process is allowlisted (browser, JIT application)\n";
        std::wcout << L"  2. Process has no RWX memory regions\n";
        std::wcout << L"  3. Insufficient permissions (try running as Administrator)\n";
        std::wcout << L"  4. Target PID is invalid or process terminated\n";
    }
    else
    {
        std::wcout << L"TEST PASSED: RWX Injection Detected!\n\n";
        std::wcout << L"Detection Details:\n";
        std::wcout << detectionResult;
        
        std::wcout << L"\n========================================\n";
        std::wcout << L"Verification Instructions:\n";
        std::wcout << L"1. Compare the Base Address above with the address\n";
        std::wcout << L"   shown in MockMalwareSim console.\n";
        std::wcout << L"2. If they match, the scanner is working correctly!\n";
        std::wcout << L"========================================\n";
    }
    
    std::wcout << std::endl;
}

void ProcessManager::StartContinuousMonitoring(std::atomic<bool>& running)
{
    std::wcout << L"\n========================================\n";
    std::wcout << L"  Continuous Process Monitoring\n";
    std::wcout << L"========================================\n\n";
    std::wcout << L"[*] Taking initial snapshot...\n";
    
    // Lấy danh sách process ban đầu
    auto previousProcesses = GetRunningProcesses();
    
    std::wcout << L"[*] Monitoring started. Press 'Q' to stop.\n\n";
    
    // Vòng lặp giám sát liên tục
    while (running)
    {
        // Ngủ 1 giây để không ngốn CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        // Lấy danh sách process mới
        auto currentProcesses = GetRunningProcesses();
        
        // === A. Phát hiện PROCESS MỚI ===
        for (const auto& pair : currentProcesses)
        {
            DWORD pid = pair.first;
            // Nếu PID này không có trong danh sách cũ -> Process mới!
            if (previousProcesses.find(pid) == previousProcesses.end())
            {
                std::wcout << L"[+] NEW PROCESS: " << pair.second.processName
                           << L" (PID: " << pid << L")" << std::endl;
                
                // Quét ngay lập tức để phát hiện injection
                std::wstring scanResult = ScanProcessMemory(pid);
                if (!scanResult.empty())
                {
                    std::wcout << L"    [!!!] MALWARE DETECTED (Injection Risk)!\n";
                    std::wcout << L"    " << scanResult;
                }
            }
        }
        
        // === B. Phát hiện PROCESS ĐÃ TẮT ===
        for (const auto& pair : previousProcesses)
        {
            DWORD pid = pair.first;
            // Nếu PID cũ này không có trong danh sách mới -> Process đã tắt
            if (currentProcesses.find(pid) == currentProcesses.end())
            {
                std::wcout << L"[-] TERMINATED: " << pair.second.processName
                           << L" (PID: " << pid << L")" << std::endl;
            }
        }
        
        // Cập nhật danh sách cũ = mới cho vòng lặp tiếp theo
        previousProcesses = currentProcesses;
    }
    
    std::wcout << L"\n[*] Continuous monitoring stopped.\n";
}

