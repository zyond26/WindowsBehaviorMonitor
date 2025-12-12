#include "RegistryMonitor.h"
#include "StartupMonitor.h"
#include "Logger.h"
#include <thread>
#include <csignal>
#include <atomic>

std::atomic<bool> g_stop{ false };

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_CLOSE_EVENT ||
        dwCtrlType == CTRL_SHUTDOWN_EVENT) {
        g_stop.store(true);
        return TRUE;
    }
    return FALSE;
}

int wmain() {
    Logger::Instance().SetLogFile(L"logs/pfm_registry.log");
    Logger::Instance().Info(L"PFM Monitoring starting...");

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    RegistryMonitor registry;
    StartupMonitor startup;

    registry.PrintBaseline();
    auto files = startup.ListStartupFiles();
    Logger::Instance().Info(L"Startup folder contents:");
    if (files.empty()) Logger::Instance().Info(L"  (empty)");
    else {
        for (auto& f : files) Logger::Instance().Info(L"  " + f);
    }

    std::thread tReg([&]() { registry.Start(); });
    std::thread tStart([&]() { startup.Start(); });

    while (!g_stop.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    Logger::Instance().Info(L"Shutdown requested, stopping monitors...");
    registry.Stop();
    startup.Stop();

    if (tReg.joinable()) tReg.join();
    if (tStart.joinable()) tStart.join();

    Logger::Instance().Info(L"PFM Monitoring stopped. Goodbye.");
    return 0;
}
