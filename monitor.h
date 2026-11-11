#ifndef SYSTEM_SOURCE_MONITOR_H
#define SYSTEM_SOURCE_MONITOR_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

#define MAX_PROCESS_ROWS 32
#define DEFAULT_POLL_INTERVAL_MS 500U

typedef struct ProcessInfo {
    DWORD process_id;
    DWORD parent_process_id;
    DWORD thread_count;
    LONG  base_priority;
    char  executable[MAX_PATH];
} ProcessInfo;

typedef struct SystemMetrics {
    double cpu_usage_percent;
    DWORD memory_load_percent;
    ULONGLONG total_physical_bytes;
    ULONGLONG available_physical_bytes;

    DWORD process_count;
    ProcessInfo processes[MAX_PROCESS_ROWS];

    ULONGLONG volume_total_bytes;
    ULONGLONG volume_free_bytes;
    BOOL volume_query_ok;

    ULONGLONG io_read_bytes;
    ULONGLONG io_write_bytes;
    ULONGLONG io_read_operations;
    ULONGLONG io_write_operations;
    BOOL io_query_ok;

    ULONGLONG sample_number;
    DWORD last_error;
} SystemMetrics;

typedef struct MonitorContext {
    CRITICAL_SECTION lock;
    HANDLE worker_thread;
    HANDLE stop_event;
    DWORD poll_interval_ms;
    char drive_root[4];
    SystemMetrics metrics;
    BOOL initialized;
} MonitorContext;

BOOL CpuMonitor_Read(double *usage_percent);
BOOL MemoryMonitor_Read(SystemMetrics *metrics);
DWORD ProcessSnapshot_Read(ProcessInfo *items, DWORD capacity);
BOOL DiskMonitor_Read(const char *drive_root, SystemMetrics *metrics);
BOOL ThreadSync_Initialize(MonitorContext *context);
void ThreadSync_Destroy(MonitorContext *context);
void ThreadSync_Publish(MonitorContext *context, const SystemMetrics *metrics);
void ThreadSync_Copy(MonitorContext *context, SystemMetrics *metrics);

BOOL Monitor_Initialize(MonitorContext *context, DWORD interval_ms,
                        const char *drive_root);
BOOL Monitor_Start(MonitorContext *context);
void Monitor_Stop(MonitorContext *context);
BOOL Monitor_GetMetrics(MonitorContext *context, SystemMetrics *metrics);

#endif
