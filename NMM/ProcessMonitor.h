#pragma once
#include "../Common/EventStruct.h"
#include <set>
#include <string>

// Check for new processes
bool CheckNewProcess(SysEvent& eventOut);

// Initialize process monitoring
void InitProcessMonitoring();

// Get all current processes
std::set<std::wstring> GetCurrentProcesses();
