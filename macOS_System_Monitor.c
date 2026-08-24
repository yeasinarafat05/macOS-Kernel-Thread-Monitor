#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

// Worker 1: Mach Virtual Memory System Call
void* collect_memory_stats(void* arg) {
    mach_port_t host = mach_host_self();
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    
    if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
        pthread_mutex_lock(&print_mutex);
        printf("[Thread 1 | Memory Syscall] Free Pages: %u | Active Pages: %u | Pageins: %llu\n",
               (unsigned int)vm_stat.free_count, 
               (unsigned int)vm_stat.active_count, 
               (unsigned long long)vm_stat.pageins);
        pthread_mutex_unlock(&print_mutex);
    }
    mach_port_deallocate(mach_task_self(), host);
    pthread_exit(NULL);
}

// Worker 2: Mach CPU Load System Call
void* collect_cpu_stats(void* arg) {
    mach_port_t host = mach_host_self();
    host_cpu_load_info_data_t cpu_info;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    
    if (host_statistics(host, HOST_CPU_LOAD_INFO, (host_info_t)&cpu_info, &count) == KERN_SUCCESS) {
        pthread_mutex_lock(&print_mutex);
        printf("[Thread 2 | CPU Syscall] User Ticks: %u | System Ticks: %u | Idle Ticks: %u\n",
               cpu_info.cpu_ticks[CPU_STATE_USER],
               cpu_info.cpu_ticks[CPU_STATE_SYSTEM],
               cpu_info.cpu_ticks[CPU_STATE_IDLE]);
        pthread_mutex_unlock(&print_mutex);
    }
    mach_port_deallocate(mach_task_self(), host);
    pthread_exit(NULL);
}

// Worker 3: BSD Kernel Sysctl Call
void* collect_sysctl_stats(void* arg) {
    struct loadavg load;
    size_t size = sizeof(load);
    int ncpu = 0;
    size_t ncpu_size = sizeof(ncpu);

    sysctlbyname("hw.ncpu", &ncpu, &ncpu_size, NULL, 0);
    if (sysctlbyname("vm.loadavg", &load, &size, NULL, 0) == 0) {
        double l1 = (double)load.ldavg[0] / load.fscale;
        double l5 = (double)load.ldavg[1] / load.fscale;
        
        pthread_mutex_lock(&print_mutex);
        printf("[Thread 3 | Sysctl Call] Hardware Cores: %d | Load Avg (1m, 5m): %.2f, %.2f\n",
               ncpu, l1, l5);
        pthread_mutex_unlock(&print_mutex);
    }
    pthread_exit(NULL);
}

// Worker 4: Mach Task Statistics Call
void* collect_task_stats(void* arg) {
    task_basic_info_data_t t_info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;

    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &count) == KERN_SUCCESS) {
        pthread_mutex_lock(&print_mutex);
        printf("[Thread 4 | Task Syscall] Task Resident Size: %lu KB | Virtual Size: %lu KB\n",
               t_info.resident_size / 1024, t_info.virtual_size / 1024);
        pthread_mutex_unlock(&print_mutex);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t worker_pool[4];

    printf("Initializing Multi-System-Call Diagnostic Engine on macOS...\n\n");

    pthread_create(&worker_pool[0], NULL, collect_memory_stats, NULL);
    pthread_create(&worker_pool[1], NULL, collect_cpu_stats, NULL);
    pthread_create(&worker_pool[2], NULL, collect_sysctl_stats, NULL);
    pthread_create(&worker_pool[3], NULL, collect_task_stats, NULL);

    for (int i = 0; i < 4; i++) {
        pthread_join(worker_pool[i], NULL);
    }

    pthread_mutex_destroy(&print_mutex);
    printf("\nAll 4 kernel subsystems sampled concurrently.\n");
    return 0;
}
