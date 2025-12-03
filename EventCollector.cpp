#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#include "EventStruct.h"
#include <ws2tcpip.h>


std::string Now()
{
    SYSTEMTIME t;
    GetLocalTime(&t);
    char buf[64];
    sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return buf;
}

SOCKET g_sock = INVALID_SOCKET;

bool InitSocket()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sock == INVALID_SOCKET) return false;

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5050);
    //server.sin_addr.s_addr = inet_addr("127.0.0.1");
    InetPton(AF_INET, L"127.0.0.1", &server.sin_addr);


    if (connect(g_sock, (sockaddr*)&server, sizeof(server)) < 0)
        return false;

    return true;
}

void CloseSocket()
{
    closesocket(g_sock);
    WSACleanup();
}

void SendEvent(const SysEvent& e)
{
    std::string msg = e.time + "|" + e.type + "|" + e.detail;
    send(g_sock, msg.c_str(), msg.size(), 0);
}

bool CheckNewProcess(SysEvent& e);
bool MonitorDirectory(SysEvent& e);
// bool MonitorTCPConnections(SysEvent& e); // test sauu

int main()
{
    if (!InitSocket())
    {
        std::cout << "Không thể kết nối GUI!\n";
        return 0;
    }

    std::cout << "=== EventCollector running ===\n";

    while (true)
    {
        {
            SysEvent ev;
            if (CheckNewProcess(ev))
            {
                ev.time = Now();
                ev.type = "Process";
                SendEvent(ev);
                std::cout << "[Process] " << ev.detail << "\n";
            }
        }

        {
            SysEvent ev;
            if (MonitorDirectory(ev))
            {
                ev.time = Now();
                ev.type = "File";
                SendEvent(ev);
                std::cout << "[File] " << ev.detail << "\n";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    CloseSocket();
    return 0;
}
