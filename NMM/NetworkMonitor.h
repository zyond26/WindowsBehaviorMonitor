#pragma once
#include <string>
#include <set>
#include "../Common/EventStruct.h"

// ========== Network Monitoring ==========

// Monitor TCP connections and detect new connections
bool MonitorTCPConnections(SysEvent& eventOut);

// Initialize network monitoring (optional)
void InitializeNetworkMonitor();

// Display current TCP connections
void DisplayCurrentConnections();

// ========== Process Monitoring ==========

// Check for new processes
bool CheckNewProcess(SysEvent& eventOut);

// Initialize process monitoring
void InitProcessMonitoring();

// Get all current processes
std::set<std::wstring> GetCurrentProcesses();
