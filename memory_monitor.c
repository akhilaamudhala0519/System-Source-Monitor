#include "monitor.h"
#include <string.h>

BOOL MemoryMonitor_Read(SystemMetrics *metrics)
{
    MEMORYSTATUSEX status;

    if (metrics == NULL) {
        return FALSE;
    }

    ZeroMemory(&status, sizeof(status));
    status.dwLength = sizeof(status);

    if (!GlobalMemoryStatusEx(&status)) {
        return FALSE;
    }

    metrics->memory_load_percent = status.dwMemoryLoad;
    metrics->total_physical_bytes = status.ullTotalPhys;
    metrics->available_physical_bytes = status.ullAvailPhys;
    return TRUE;
}
