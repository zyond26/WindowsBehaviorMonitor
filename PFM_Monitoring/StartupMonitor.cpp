#include "StartupMonitor.h"
#include "Logger.h"
#include <filesystem>
#include <windows.h>

StartupMonitor::StartupMonitor() {
    try {
        startupPath_ = GetStartupPathInternal();
    }
    catch (...) {
        startupPath_.clear();
    }
}

StartupMonitor::~StartupMonitor() {
    Stop();
}

std::wstring StartupMonitor::GetStartupPathInternal() {
    wchar_t buf[MAX_PATH];
    if (ExpandEnvironmentStringsW(L"%APPDATA%\\Microsoft\\Windows\\Start Menu\\Programs\\Startup", buf, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to get startup path");
    }
    return std::wstring(buf);
}

std::vector<std::wstring> StartupMonitor::ListStartupFiles() const {
    std::vector<std::wstring> out;
    if (startupPath_.empty()) return out;

    try {
        for (auto& entry : std::filesystem::directory_iterator(startupPath_)) {
            out.push_back(entry.path().filename().wstring());
        }
    }
    catch (...) {
    }
    return out;
}

void StartupMonitor::Start() {
    if (startupPath_.empty()) {
        Logger::Instance().Warn(L"StartupMonitor: startup path unavailable; not monitoring.");
        return;
    }

    running_.store(true);
    Logger::Instance().Info(L"StartupMonitor: Monitoring folder: " + startupPath_);

    HANDLE hDir = CreateFileW(
        startupPath_.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (hDir == INVALID_HANDLE_VALUE) {
        Logger::Instance().Error(L"StartupMonitor: CreateFileW failed to open startup folder.");
        return;
    }

    const DWORD bufSize = 16 * 1024;
    std::vector<BYTE> buffer(bufSize);
    OVERLAPPED overlapped{};
    HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    overlapped.hEvent = hEvent;

    while (running_.load()) {
        ResetEvent(hEvent);
        DWORD bytesReturned = 0;
        BOOL ok = ReadDirectoryChangesW(
            hDir,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE,
            &bytesReturned,
            &overlapped,
            nullptr);

        if (!ok) {
            DWORD err = GetLastError();
            if (err != ERROR_IO_PENDING) {
                Logger::Instance().Error(L"StartupMonitor: ReadDirectoryChangesW failed with error: " + std::to_wstring(err));
                break;
            }
        }

        DWORD wait = WaitForSingleObject(hEvent, 500);
        if (!running_.load()) break;

        if (wait == WAIT_OBJECT_0) {
            DWORD bytes = 0;
            if (!GetOverlappedResult(hDir, &overlapped, &bytes, FALSE)) {
                Logger::Instance().Warn(L"StartupMonitor: GetOverlappedResult failed");
                continue;
            }

            BYTE* ptr = buffer.data();
            while (true) {
                FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);
                std::wstring file(info->FileName, info->FileNameLength / sizeof(WCHAR));
                switch (info->Action) {
                case FILE_ACTION_ADDED:
                    Logger::Instance().Warn(L"[ALERT] Startup File Added: " + file);
                    break;
                case FILE_ACTION_REMOVED:
                    Logger::Instance().Warn(L"[ALERT] Startup File Removed: " + file);
                    break;
                case FILE_ACTION_MODIFIED:
                    Logger::Instance().Warn(L"[ALERT] Startup File Modified: " + file);
                    break;
                default:
                    Logger::Instance().Info(L"[Startup] Action: " + std::to_wstring(info->Action) + L" File: " + file);
                    break;
                }

                if (info->NextEntryOffset == 0) break;
                ptr += info->NextEntryOffset;
            }
        }
    }

    CloseHandle(hEvent);
    CloseHandle(hDir);

    Logger::Instance().Info(L"StartupMonitor: Stopped.");
}

void StartupMonitor::Stop() {
    running_.store(false);
}
