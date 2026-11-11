#include "monitor.h"
#include <tlhelp32.h>
#include <string.h>

DWORD ProcessSnapshot_Read(ProcessInfo *items, DWORD capacity)
{
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    DWORD count = 0;

    if (items == NULL || capacity == 0) {
        return 0;
    }

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(entry);

    if (Process32First(snapshot, &entry)) {
        do {
            if (count >= capacity) {
                break;
            }

            items[count].process_id = entry.th32ProcessID;
            items[count].parent_process_id = entry.th32ParentProcessID;
            items[count].thread_count = entry.cntThreads;
            items[count].base_priority = entry.pcPriClassBase;

#ifdef UNICODE
            WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1,
                                items[count].executable, MAX_PATH,
                                NULL, NULL);
#else
            strncpy_s(items[count].executable, MAX_PATH,
                      entry.szExeFile, _TRUNCATE);
#endif
            ++count;
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return count;
}
