#include <windows.h>
#include <tlhelp32.h>
#include "EventStruct.h"
#include <set>
#include <string>

std::set<std::wstring> g_knownProcesses;

// Lấy process đang chạy
void SnapshotProcesses(std::set<std::wstring>& out)
{
    out.clear();
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(snap, &pe)) {
        do {
            out.insert(pe.szExeFile);
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
}

// Kiểm tra process mới
bool CheckNewProcess(SysEvent& eventOut)
{
    std::set<std::wstring> current;
    SnapshotProcesses(current);

    for (auto& p : current)
    {
        if (!g_knownProcesses.count(p))
        {
            // Process mới xuất hiện
            g_knownProcesses.insert(p);

            eventOut.type = "Process";
            eventOut.time = "TODO";               // sẽ set time ở main
            eventOut.detail = std::string(p.begin(), p.end());
            return true;
        }
    }

    return false;
}
