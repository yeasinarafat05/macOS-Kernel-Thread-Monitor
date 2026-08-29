# Optimized Multi-Threaded macOS Kernel Telemetry Engine

An optimized system monitoring tool written in C for macOS that parallelizes kernel statistics sampling using POSIX threads (`pthread`) and native macOS XNU Mach/BSD system calls.

## Features & System Calls Used
- **Thread 1 (Mach Memory Subsystem):** Queries `host_statistics64` for active, free pages, and pageins.
- **Thread 2 (Mach Scheduler):** Queries `host_statistics` for User, System, and Idle CPU ticks.
- **Thread 3 (BSD Kernel):** Queries `sysctlbyname` for hardware core count and system load average.
- **Thread 4 (Mach Task Subsystem):** Queries `task_info` for resident memory footprint.
- 
## Project Demo
[🎥 Watch Project Demo on  https://www.youtube.com/watch?v=J4D8xnK4xkE]

## How to Build & Run
```bash
gcc -O3 macOS_System_Monitor.c -o system_monitor
./system_monitor
