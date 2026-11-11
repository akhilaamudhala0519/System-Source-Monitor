#include "monitor.h"

BOOL DiskMonitor_Read(const char *drive_root, SystemMetrics *metrics)
{
    ULARGE_INTEGER available_to_caller;
    ULARGE_INTEGER total_bytes;
    ULARGE_INTEGER total_free_bytes;

    if (drive_root == NULL || metrics == NULL) {
        return FALSE;
    }

    if (!GetDiskFreeSpaceExA(drive_root, &available_to_caller,
                             &total_bytes, &total_free_bytes)) {
        return FALSE;
    }

    metrics->volume_total_bytes = total_bytes.QuadPart;
    /* Use caller-available free bytes to respect volume quotas. */
    metrics->volume_free_bytes = available_to_caller.QuadPart;
    metrics->volume_query_ok = TRUE;
    return TRUE;
}
