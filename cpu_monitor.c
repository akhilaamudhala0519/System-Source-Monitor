#include "monitor.h"

static ULONGLONG FileTimeToUInt64(const FILETIME *value)
{
    ULARGE_INTEGER converted;
    converted.LowPart = value->dwLowDateTime;
    converted.HighPart = value->dwHighDateTime;
    return converted.QuadPart;
}

BOOL CpuMonitor_Read(double *usage_percent)
{
    static ULONGLONG previous_idle = 0;
    static ULONGLONG previous_kernel = 0;
    static ULONGLONG previous_user = 0;
    static BOOL has_baseline = FALSE;

    FILETIME idle_time, kernel_time, user_time;
    ULONGLONG idle, kernel, user;
    ULONGLONG idle_delta, total_delta;

    if (usage_percent == NULL) {
        return FALSE;
    }

    if (!GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
        return FALSE;
    }

    idle = FileTimeToUInt64(&idle_time);
    kernel = FileTimeToUInt64(&kernel_time);
    user = FileTimeToUInt64(&user_time);

    if (!has_baseline) {
        previous_idle = idle;
        previous_kernel = kernel;
        previous_user = user;
        has_baseline = TRUE;
        *usage_percent = 0.0;
        return TRUE;
    }

    /* Kernel time includes idle time on Windows. */
    idle_delta = idle - previous_idle;
    total_delta = (kernel - previous_kernel) + (user - previous_user);

    previous_idle = idle;
    previous_kernel = kernel;
    previous_user = user;

    if (total_delta == 0) {
        *usage_percent = 0.0;
        return TRUE;
    }

    if (idle_delta > total_delta) {
        idle_delta = total_delta;
    }

    *usage_percent = 100.0 *
        (double)(total_delta - idle_delta) / (double)total_delta;

    if (*usage_percent < 0.0) *usage_percent = 0.0;
    if (*usage_percent > 100.0) *usage_percent = 100.0;
    return TRUE;
}
