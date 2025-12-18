#include <windows.h>
#include "EventStruct.h"
#include <string>

bool MonitorDirectory(SysEvent& eventOut)
{
    static HANDLE hDir = INVALID_HANDLE_VALUE;

    if (hDir == INVALID_HANDLE_VALUE)
    {
        hDir = CreateFile(
            L"C:\\TestWatch",
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );
        if (hDir == INVALID_HANDLE_VALUE)
            return false;
    }

    BYTE buffer[1024];
    DWORD bytesReturned;

    BOOL ok = ReadDirectoryChangesW(
        hDir,
        &buffer,
        sizeof(buffer),
        TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE,
        &bytesReturned,
        NULL,
        NULL
    );

    if (!ok) return false;

    FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer;

    std::wstring file(fni->FileName, fni->FileNameLength / sizeof(WCHAR));

    eventOut.type = "File";
    eventOut.time = "TODO";     
    eventOut.detail = std::string(file.begin(), file.end());

    return true;
}
