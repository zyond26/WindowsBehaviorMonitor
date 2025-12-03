#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <windows.h>       
#include "EventStruct.h"

// prototypes
bool CheckNewProcess(SysEvent& e);
bool MonitorDirectory(SysEvent& e);
// Nếu chưa có NetworkMonitor.cpp thì comment dòng dưới
//bool MonitorTCPConnections(SysEvent& e);

// Hàm lấy thời gian
std::string Now()
{
    SYSTEMTIME t;
    GetLocalTime(&t);

    char buf[64];
    sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

    return std::string(buf);
}

int main()
{
    std::cout << "=== EventCollector running ===\n";

    while (true)
    {
        SysEvent ev;

        // --- Process event ---
        if (CheckNewProcess(ev))
        {
            ev.time = Now();
            std::cout << "[Process] " << ev.detail << "\n";
        }

        // --- File event ---
        if (MonitorDirectory(ev))
        {
            ev.time = Now();
            std::cout << "[File] " << ev.detail << "\n";
        }

        // --- Network event ---
       /* if (MonitorTCPConnections(ev))
        {
            ev.time = Now();
            std::cout << "[Network] " << ev.detail << "\n";
        }*/

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}
