#include <windows.h>
#include <tlhelp32.h>
#include "EventStruct.h"
#include <set>
#include <string>

std::set<std::wstring> g_knownProcesses;

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

bool CheckNewProcess(SysEvent& eventOut)
{
    std::set<std::wstring> current;
    SnapshotProcesses(current);

    for (auto& p : current)
    {
        if (!g_knownProcesses.count(p))
        {
            g_knownProcesses.insert(p);

            eventOut.type = "Process";
            eventOut.time = "TODO";               
            eventOut.detail = std::string(p.begin(), p.end());
            return true;
        }
    }

    return false;
}
