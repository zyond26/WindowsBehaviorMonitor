#include "RegistryMonitor.h"
#include "Logger.h"
#include <iostream>

#define RUN_SUBKEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

static std::wstring ToWString(const std::wstring & s) { return s; }

RegistryMonitor::RegistryMonitor() {
    LONG r = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_SUBKEY, 0, KEY_READ | KEY_NOTIFY, &runKey_);
    if (r != ERROR_SUCCESS) {
        runKey_ = nullptr;
        Logger::Instance().Warn(L"RegistryMonitor: Cannot open HKCU Run key (no monitoring).");
    }
    notifyEvent_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    baseline_.clear();
}

RegistryMonitor::~RegistryMonitor() {
    Stop();
    if (notifyEvent_) CloseHandle(notifyEvent_);
    if (runKey_) RegCloseKey(runKey_);
}

RegistryMonitor::RegMap RegistryMonitor::TakeSnapshot(HKEY rootKey) {
    RegMap result;
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(rootKey, RUN_SUBKEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return result;
    }

    DWORD index = 0;
    WCHAR name[512];
    BYTE data[4096];
    DWORD nameSize = 0, dataSize = 0, type = 0;

    while (true) {
        nameSize = _countof(name);
        dataSize = _countof(data);
        LONG res = RegEnumValueW(hKey, index, name, &nameSize, nullptr, &type, data, &dataSize);
        if (res != ERROR_SUCCESS) break;

        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            std::wstring val(reinterpret_cast<wchar_t*>(data), dataSize / sizeof(WCHAR));
            while (!val.empty() && val.back() == L'\0') val.pop_back();
            result[std::wstring(name, nameSize)] = val;
        }
        ++index;
    }
    RegCloseKey(hKey);
    return result;
}

void RegistryMonitor::PrintBaseline() const {
    Logger::Instance().Info(L"RegistryMonitor: Baseline (HKCU Run) items:");
    if (baseline_.empty()) {
        Logger::Instance().Info(L"  (empty)");
        return;
    }
    for (const auto& kv : baseline_) {
        std::wstring line = L"  " + kv.first + L" -> " + kv.second;
        Logger::Instance().Info(line);
    }
}

void RegistryMonitor::CompareAndAlert(const RegMap& current) {
    for (const auto& kv : current) {
        const auto& name = kv.first;
        const auto& val = kv.second;
        auto it = baseline_.find(name);
        if (it == baseline_.end()) {
            Logger::Instance().Warn(L"[ALERT] New Run value: " + name + L" -> " + val);
        }
        else if (it->second != val) {
            Logger::Instance().Warn(L"[ALERT] Modified Run value: " + name + L" | Old: " + it->second + L" -> New: " + val);
        }
    }
    for (const auto& kv : baseline_) {
        const auto& name = kv.first;
        if (current.find(name) == current.end()) {
            Logger::Instance().Warn(L"[ALERT] Removed Run value: " + name);
        }
    }
}

void RegistryMonitor::Start() {
    if (!runKey_) {
        baseline_ = TakeSnapshot(HKEY_CURRENT_USER);
        PrintBaseline();
        Logger::Instance().Warn(L"RegistryMonitor: Monitoring disabled (runKey handle not available).");
        return;
    }

    running_.store(true);
    baseline_ = TakeSnapshot(HKEY_CURRENT_USER);
    PrintBaseline();

    while (running_.load()) {
        LONG r = RegNotifyChangeKeyValue(
            runKey_,
            FALSE,
            REG_NOTIFY_CHANGE_LAST_SET,
            notifyEvent_,
            TRUE
        );

        if (r != ERROR_SUCCESS) {
            Logger::Instance().Error(L"RegistryMonitor: RegNotifyChangeKeyValue failed.");
            break;
        }

        DWORD wait = WaitForSingleObject(notifyEvent_, 500);
        if (wait == WAIT_OBJECT_0) {
            RegMap current = TakeSnapshot(HKEY_CURRENT_USER);
            CompareAndAlert(current);
            baseline_ = current;
        }
    }

    Logger::Instance().Info(L"RegistryMonitor: Stopped.");
}

void RegistryMonitor::Stop() {
    running_.store(false);
    if (notifyEvent_) SetEvent(notifyEvent_);
}