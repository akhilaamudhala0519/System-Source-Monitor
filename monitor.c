#include "monitor.h"
#include <string.h>

static DWORD WINAPI TelemetryWorker(LPVOID parameter)
{
    MonitorContext *context = (MonitorContext *)parameter;
    SystemMetrics current;

    ZeroMemory(&current, sizeof(current));

    while (WaitForSingleObject(context->stop_event, 0) == WAIT_TIMEOUT) {
        double cpu = 0.0;

        if (CpuMonitor_Read(&cpu)) {
            current.cpu_usage_percent = cpu;
        }
        (void)MemoryMonitor_Read(&current);

        current.process_count =
            ProcessSnapshot_Read(current.processes, MAX_PROCESS_ROWS);

        current.volume_query_ok = FALSE;
        if (!DiskMonitor_Read(context->drive_root, &current)) {
            current.last_error = GetLastError();
        }

        {
            IO_COUNTERS counters;
            if (GetProcessIoCounters(GetCurrentProcess(), &counters)) {
                current.io_read_bytes = counters.ReadTransferCount;
                current.io_write_bytes = counters.WriteTransferCount;
                current.io_read_operations = counters.ReadOperationCount;
                current.io_write_operations = counters.WriteOperationCount;
                current.io_query_ok = TRUE;
            } else {
                current.io_query_ok = FALSE;
                current.last_error = GetLastError();
            }
        }

        ++current.sample_number;
        ThreadSync_Publish(context, &current);

        if (WaitForSingleObject(context->stop_event,
                                context->poll_interval_ms) != WAIT_TIMEOUT) {
            break;
        }
    }

    return 0;
}

BOOL Monitor_Initialize(MonitorContext *context, DWORD interval_ms,
                        const char *drive_root)
{
    size_t i;

    if (context == NULL) {
        return FALSE;
    }

    ZeroMemory(context, sizeof(*context));
    context->poll_interval_ms = interval_ms;
    if (context->poll_interval_ms < 100U) context->poll_interval_ms = 100U;
    if (context->poll_interval_ms > 10000U) context->poll_interval_ms = 10000U;

    if (drive_root == NULL || drive_root[0] == '\0') {
        drive_root = "C:";
    }

    /* Normalize a drive designator to a root path, e.g. C:\ */
    context->drive_root[0] = drive_root[0];
    context->drive_root[1] = ':';
    context->drive_root[2] = '\\';
    context->drive_root[3] = '\0';
    for (i = 0; i < 3; ++i) {
        if (context->drive_root[i] == '\0') {
            return FALSE;
        }
    }

    if (!ThreadSync_Initialize(context)) {
        return FALSE;
    }

    context->stop_event = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (context->stop_event == NULL) {
        ThreadSync_Destroy(context);
        return FALSE;
    }

    return TRUE;
}

BOOL Monitor_Start(MonitorContext *context)
{
    if (context == NULL || context->stop_event == NULL) {
        return FALSE;
    }

    context->worker_thread = CreateThread(NULL, 0, TelemetryWorker,
                                         context, 0, NULL);
    return context->worker_thread != NULL;
}

void Monitor_Stop(MonitorContext *context)
{
    if (context == NULL) {
        return;
    }

    if (context->stop_event != NULL) {
        SetEvent(context->stop_event);
    }
    if (context->worker_thread != NULL) {
        WaitForSingleObject(context->worker_thread, INFINITE);
        CloseHandle(context->worker_thread);
        context->worker_thread = NULL;
    }
    if (context->stop_event != NULL) {
        CloseHandle(context->stop_event);
        context->stop_event = NULL;
    }
    ThreadSync_Destroy(context);
}

BOOL Monitor_GetMetrics(MonitorContext *context, SystemMetrics *metrics)
{
    if (context == NULL || metrics == NULL || !context->initialized) {
        return FALSE;
    }
    ThreadSync_Copy(context, metrics);
    return TRUE;
}
