#pragma once
#include <map>
#include <string>
#include <windows.h>
#include <atomic>

class RegistryMonitor {
public:
    using RegMap = std::map<std::wstring, std::wstring>;

    RegistryMonitor();
    ~RegistryMonitor();

    void Start();
    void Stop();

    void PrintBaseline() const;

private:
    RegMap TakeSnapshot(HKEY rootKey);
    void CompareAndAlert(const RegMap& current);

    HKEY runKey_ = nullptr;    
    RegMap baseline_;
    HANDLE notifyEvent_ = nullptr;
    std::atomic<bool> running_{ false };
};
