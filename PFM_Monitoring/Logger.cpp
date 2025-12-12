#include "Logger.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include "Windows.h"

Logger& Logger::Instance() {
    static Logger inst;
    return inst;
}

void Logger::SetLogFile(const std::wstring& path) {
    std::lock_guard<std::mutex> lock(mtx_);
    logfile_ = path;
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) std::filesystem::create_directories(p.parent_path());
    }
    catch (...) {}
}

static std::wstring TimestampNow() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &t);
    wchar_t buf[64];
    std::wcsftime(buf, sizeof(buf) / sizeof(wchar_t), L"%Y-%m-%dT%H:%M:%S", &tm);
    return std::wstring(buf);
}

void Logger::Write(const std::wstring& level, const std::wstring& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::wstring line = L"[" + TimestampNow() + L"] [" + level + L"] " + msg + L"\r\n";

    std::wcout << line;

    if (!logfile_.empty()) {
        HANDLE h = CreateFileW(
            logfile_.c_str(),
            FILE_APPEND_DATA | GENERIC_WRITE,
            FILE_SHARE_READ,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (h == INVALID_HANDLE_VALUE) {
            return;
        }

        LARGE_INTEGER fileSize{};
        if (GetFileSizeEx(h, &fileSize)) {
            if (fileSize.QuadPart == 0) {
                WORD bom = 0xFEFF;
                DWORD written = 0;
                WriteFile(h, &bom, sizeof(bom), &written, nullptr);
            }
        }

        SetFilePointer(h, 0, nullptr, FILE_END);

        DWORD bytesToWrite = static_cast<DWORD>(line.size() * sizeof(wchar_t));
        DWORD bytesWritten = 0;
        BOOL ok = WriteFile(h, reinterpret_cast<const BYTE*>(line.c_str()), bytesToWrite, &bytesWritten, nullptr);

        (void)ok;

        CloseHandle(h);
    }
}

void Logger::Info(const std::wstring& msg) { Write(L"INFO", msg); }
void Logger::Warn(const std::wstring& msg) { Write(L"WARN", msg); }
void Logger::Error(const std::wstring& msg) { Write(L"ERROR", msg); }

Logger::~Logger() {}
