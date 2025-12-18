#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <vector>    
#include <string>    
#include "EventStruct.h"


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

using namespace std;  
vector<string> listCu;

bool MonitorTCPConnections(SysEvent& eventOut) {
    DWORD dwSize = 0;
    GetExtendedTcpTable(
        NULL, 
        &dwSize,
        TRUE, 
        AF_INET,
        TCP_TABLE_OWNER_PID_ALL,
        0);

    PMIB_TCPTABLE_OWNER_PID bangKetNoi = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);

    DWORD ketQua = GetExtendedTcpTable(
        bangKetNoi, 
        &dwSize,
        TRUE, AF_INET, 
        TCP_TABLE_OWNER_PID_ALL,
        0);

    if (ketQua != 0)
    {
        free(bangKetNoi);
        return false;
    }


    for (DWORD i = 0; i < bangKetNoi->dwNumEntries; i++)
    {
        MIB_TCPROW_OWNER_PID dongHienTai = bangKetNoi->table[i];
        struct in_addr ipChua = { dongHienTai.dwRemoteAddr }; //struct la {}

        char* ip = inet_ntoa(ipChua);
        int port = ntohs(dongHienTai.dwRemotePort);
        DWORD pid = dongHienTai.dwOwningPid;
        
        string info = string(ip) + ":" + to_string(port) + " (PID: " + to_string(pid) + ")";

        bool laMoi = true;
        for (int k = 0; k < listCu.size(); k++) {
            if (listCu[k] == info) {
                laMoi = false; 
                break;
            }
        }
        if (laMoi) {      
            listCu.push_back(info);
            eventOut.type = "Network";
            eventOut.detail = info;

            free(bangKetNoi);
            return true; 
        }

    }
    free(bangKetNoi); 
    return false;
}