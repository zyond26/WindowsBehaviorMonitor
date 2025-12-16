#pragma once
#include <string>
#include <vector>
#include <atomic>

class StartupMonitor {
public:
    StartupMonitor();
    ~StartupMonitor();

    void Start();
    void Stop();

    std::vector<std::wstring> ListStartupFiles() const;

private:
    std::wstring startupPath_;
    std::atomic<bool> running_{ false };

    static std::wstring GetStartupPathInternal();
};