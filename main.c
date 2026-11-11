#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double BytesToGiB(ULONGLONG bytes)
{
    return (double)bytes / (1024.0 * 1024.0 * 1024.0);
}

static void PrintDashboard(const SystemMetrics *m, const char *drive)
{
    DWORD i;
    ULONGLONG used;

    system("cls");
    printf("============================================================\n");
    printf("                 SYSTEM SOURCE MONITOR\n");
    printf("============================================================\n");
    printf("Sample: %llu\n\n", (unsigned long long)m->sample_number);

    printf("[CPU]\n");
    printf("System utilization : %6.2f%%\n\n", m->cpu_usage_percent);

    printf("[MEMORY]\n");
    printf("Physical RAM load  : %6lu%%\n", (unsigned long)m->memory_load_percent);
    printf("Total RAM          : %8.2f GiB\n", BytesToGiB(m->total_physical_bytes));
    printf("Available RAM      : %8.2f GiB\n\n", BytesToGiB(m->available_physical_bytes));

    printf("[VOLUME %s]\n", drive);
    if (m->volume_query_ok && m->volume_total_bytes > 0) {
        used = m->volume_total_bytes - m->volume_free_bytes;
        printf("Total capacity     : %8.2f GiB\n", BytesToGiB(m->volume_total_bytes));
        printf("Available to user  : %8.2f GiB\n", BytesToGiB(m->volume_free_bytes));
        printf("Used (approx.)     : %8.2f GiB\n\n", BytesToGiB(used));
    } else {
        printf("Volume query failed (error %lu)\n\n", (unsigned long)m->last_error);
    }

    printf("[THIS PROCESS - I/O COUNTERS]\n");
    if (m->io_query_ok) {
        printf("Read bytes         : %llu\n", (unsigned long long)m->io_read_bytes);
        printf("Write bytes        : %llu\n", (unsigned long long)m->io_write_bytes);
        printf("Read operations    : %llu\n", (unsigned long long)m->io_read_operations);
        printf("Write operations   : %llu\n\n", (unsigned long long)m->io_write_operations);
    } else {
        printf("I/O counters unavailable\n\n");
    }

    printf("[PROCESS SNAPSHOT - first %lu entries]\n",
           (unsigned long)m->process_count);
    printf("%-7s %-7s %-7s %-7s %s\n", "PID", "PPID", "Threads", "Priority", "Executable");
    for (i = 0; i < m->process_count; ++i) {
        printf("%-7lu %-7lu %-7lu %-7ld %s\n",
               (unsigned long)m->processes[i].process_id,
               (unsigned long)m->processes[i].parent_process_id,
               (unsigned long)m->processes[i].thread_count,
               (long)m->processes[i].base_priority,
               m->processes[i].executable);
    }

    printf("\nRefreshing automatically. Press Ctrl+C to exit.\n");
}

int main(int argc, char **argv)
{
    MonitorContext context;
    SystemMetrics metrics;
    DWORD interval = DEFAULT_POLL_INTERVAL_MS;
    const char *drive = "C:";

    if (argc >= 2) {
        unsigned long parsed = strtoul(argv[1], NULL, 10);
        if (parsed > 0) interval = (DWORD)parsed;
    }
    if (argc >= 3) {
        drive = argv[2];
    }

    if (!Monitor_Initialize(&context, interval, drive)) {
        fprintf(stderr, "Failed to initialize monitor (error %lu).\n",
                (unsigned long)GetLastError());
        return 1;
    }

    if (!Monitor_Start(&context)) {
        fprintf(stderr, "Failed to start telemetry thread (error %lu).\n",
                (unsigned long)GetLastError());
        Monitor_Stop(&context);
        return 1;
    }

    for (;;) {
        Sleep(500);
        ZeroMemory(&metrics, sizeof(metrics));
        if (Monitor_GetMetrics(&context, &metrics)) {
            PrintDashboard(&metrics, context.drive_root);
        }
    }

    /* The console loop runs until Ctrl+C/process termination. */
    Monitor_Stop(&context);
    return 0;
}
