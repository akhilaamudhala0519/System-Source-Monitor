# System Source Monitor

A lightweight Windows system telemetry monitor written in native C using the Win32 API.

The project follows the architecture described in the presentation: Win32 system queries feed a C aggregation engine, while a dedicated worker thread periodically refreshes metrics. The current implementation is a console dashboard; the collection modules are separated so a GUI can be added later.

## Features

- **CPU utilization:** calculates total CPU usage from two `GetSystemTimes()` samples.
- **Memory telemetry:** reports physical RAM total, available memory, and usage percentage using `GlobalMemoryStatusEx()`.
- **Process snapshot:** enumerates running processes with PID, parent PID, thread count, and base priority using Toolhelp32.
- **Volume storage:** reports total and free capacity for a selected drive using `GetDiskFreeSpaceExA()`.
- **Process I/O:** reads byte and operation counters for the monitor process using `GetProcessIoCounters()`.
- **Background polling:** a Win32 worker thread refreshes the metrics at a configurable interval.
- **Thread-safe shared state:** protects published metrics with a Win32 `CRITICAL_SECTION`.

## Project structure

```text
System_Source_Monitor/
├── README.md
├── .gitignore
└── src/
    ├── main.c
    ├── monitor.h
    ├── monitor.c
    ├── cpu_monitor.c
    ├── memory_monitor.c
    ├── process_snapshot.c
    ├── disk_monitor.c
    └── thread_sync.c
```

## Requirements

- Windows 10 or later (Windows 7 may work, but is not tested)
- Microsoft Visual Studio 2019/2022 with the **Desktop development with C++** workload, or another C compiler with Windows SDK headers and libraries
- Win32 headers/libraries: `Windows.h`, `TlHelp32.h`, `Psapi` is not required

## Build with MSVC

Open **Developer Command Prompt for Visual Studio**, change to this project directory, and run:

```bat
cl /nologo /W4 /O2 /TC src\main.c src\monitor.c src\cpu_monitor.c src\memory_monitor.c src\process_snapshot.c src\disk_monitor.c src\thread_sync.c /Fe:SystemSourceMonitor.exe
```

Run:

```bat
SystemSourceMonitor.exe
```

Optional arguments:

```bat
SystemSourceMonitor.exe [poll_interval_ms] [drive]
```

Examples:

```bat
SystemSourceMonitor.exe 500 C:
SystemSourceMonitor.exe 1000 D:
```

The polling interval is clamped to 100–10000 ms. The drive argument should be a drive root such as `C:` or `D:`. If omitted, `C:` is used.

## Build with MinGW-w64

From a MinGW-w64 terminal:

```bash
gcc -std=c11 -Wall -Wextra -O2 -Isrc \
  src/main.c src/monitor.c src/cpu_monitor.c src/memory_monitor.c \
  src/process_snapshot.c src/disk_monitor.c src/thread_sync.c \
  -o SystemSourceMonitor.exe
```

## Notes and limitations

- CPU utilization is a system-wide estimate based on the difference between consecutive idle and total kernel/user time samples. The first sample is used only to initialize the baseline.
- The process list is a point-in-time snapshot. It can change while being displayed; processes that exit during enumeration are naturally absent from later snapshots.
- Process I/O counters describe **this monitor process**, not all processes or the entire disk controller.
- Disk capacity is queried for one volume root supplied on the command line. This is capacity/free-space telemetry, not live disk throughput.
- The program uses a console renderer. A graphical dashboard, service wrapper, historical charts, per-process I/O enumeration, and disk throughput sampling are possible extensions, not included features.
- The presentation mentions benchmark figures (including CPU usage and polling latency). Those values are not independently benchmarked by this source code and should not be treated as guaranteed results. Actual resource use depends on hardware, Windows version, compiler settings, and polling interval.

## Safety

This program only reads system telemetry. It does not modify system settings, terminate processes, or write to monitored volumes.

## License

Choose a license before publishing (for example, MIT) and add the corresponding `LICENSE` file if you want others to reuse the code.
