#pragma once
#include "../Common/EventStruct.h"

// Monitor file system changes in specified directory
bool MonitorDirectory(SysEvent& eventOut);

// Set directory to monitor
void SetMonitorDirectory(const wchar_t* path);
