#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <sys/syscall.h>

// total_execution_time = finish - start

// cpu_execution_time;
// release_time; 
// start_time; 
// finish_time; 
// wait_time = turnaround - execution
// response_time = start - release
// turnaround_time = finish - release
// CPU useful work
// CPU Utilization
// memory consumption


// Error handling macro
#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

// Function to get the current time in milliseconds
double get_time_ms(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6; // Convert to milliseconds
}

// Function to get memory usage of a specific thread
double get_thread_memory_usage(pthread_t thread_id) {
    char filename[256];
    FILE *f;
    char line[256];
    long rss = 0;
    
    // Get thread ID for /proc filesystem
    pid_t tid = syscall(SYS_gettid);
    
    // Construct path to status file
    snprintf(filename, sizeof(filename), "/proc/self/task/%d/status", tid);
    
    f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open status file");
        return 0.0;
    }
    
    // Look for VmRSS line in status file
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            char* p = line + 6;
            // Skip whitespace
            while (*p && (*p == ' ' || *p == '\t')) p++;
            // Parse the value
            rss = strtol(p, NULL, 10);
            break;
        }
    }
    
    fclose(f);
    return (double)rss;
}

// Structure to hold timing data
typedef struct {
    double total_execution_time; 
    double cpu_execution_time; 
    double release_time; 
    double start_time; 
    double finish_time; 
    double wait_time; 
    double response_time;
    double turnaround_time; 
    double cpu_useful_work;
    double cpu_utilization;
    double memory_consumption;
} ThreadMetrics;

ThreadMetrics thread_data[3];

// Thread 1 - Nour, Mai, Yasmeen
void *thread1(void *arg) {
    struct timespec start_cpu, end_cpu;

    // Start timers
    thread_data[0].start_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_cpu);

    char char1, char2;
    printf("Thread 1 - Enter two alphabetic characters: \n");
    scanf(" %c %c", &char1, &char2);

    for (char i = char1; i <= char2; i++) 
        printf("%c", i);
    printf("\n");

    // End timers
    thread_data[0].finish_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_cpu);

    // Store results
    thread_data[0].total_execution_time = thread_data[0].finish_time - thread_data[0].start_time;
    thread_data[0].cpu_execution_time = (end_cpu.tv_sec * 1000.0 + end_cpu.tv_nsec / 1.0e6) - (start_cpu.tv_sec * 1000.0 + start_cpu.tv_nsec / 1.0e6);
    thread_data[0].memory_consumption = get_thread_memory_usage(pthread_self());

    printf("Thread 1 has finished execution.\n \n");
    pthread_exit(NULL);
    return NULL;
}

//Thread2 - Lama, Habiba
void *thread2(void *arg) {

    printf("Thread 2\n");

    struct timespec start_cpu, end_cpu;

    thread_data[1].start_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_cpu);

    pthread_t threadID = pthread_self();
    printf("First Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Second Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Third Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Bye from thread 2!\n");
    
    thread_data[1].finish_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_cpu);

    thread_data[1].total_execution_time = thread_data[1].finish_time - thread_data[1].start_time;
    thread_data[1].cpu_execution_time = (end_cpu.tv_sec * 1000.0 + end_cpu.tv_nsec / 1.0e6) - (start_cpu.tv_sec * 1000.0 + start_cpu.tv_nsec / 1.0e6);
    thread_data[1].memory_consumption = get_thread_memory_usage(pthread_self());

    printf("Thread 2 has finished execution.\n \n");
    pthread_exit(NULL);
    return NULL;
}

// Thread 3 - Salma, Layla 
void *thread3(void *arg) {
    struct timespec start_cpu, end_cpu;

    thread_data[2].start_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_cpu);

    int num1, num2;
    printf("Thread 3 - Enter two integers: \n");
    scanf("%d %d", &num1, &num2);

    int sum = 0, prod = 1;
    int max = (num1 > num2) ? num1 : num2;
    int min = (num1 < num2) ? num1 : num2;

    float avg = 0; // Changed to float to handle division accurately
    for (int i = min; i <= max; i++) {
        sum += i;
        prod *= i;
    }

    avg = (float)sum / (max - min + 1);
    printf("Sum: %d\n", sum);
    printf("Average: %.2f\n", avg);
    printf("Product: %d\n", prod);

    thread_data[2].finish_time = get_time_ms(CLOCK_MONOTONIC);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_cpu);

    thread_data[2].total_execution_time = thread_data[2].finish_time - thread_data[2].start_time;
    thread_data[2].cpu_execution_time = (end_cpu.tv_sec * 1000.0 + end_cpu.tv_nsec / 1.0e6) - (start_cpu.tv_sec * 1000.0 + start_cpu.tv_nsec / 1.0e6);
    thread_data[2].memory_consumption = get_thread_memory_usage(pthread_self());

    printf("Thread 3 has finished execution.\n \n");
    pthread_exit(NULL);
    return NULL;
}


int main() {
    // CPU affinity initialization (optional, pins threads to CPU 0)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    // Thread attribute objects for each thread
    pthread_attr_t attr1, attr2, attr3;
    pthread_t thread_1, thread_2, thread_3;
    struct sched_param param1, param2, param3;
    int s;

    unsigned long user1, nice1, system1, idle1, iowait1, irq1, softirq1, steal1;
    unsigned long user2, nice2, system2, idle2, iowait2, irq2, softirq2, steal2;

    FILE *file = fopen("/proc/stat", "r");
    if (!file) return -1;

    char line[256];
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return -1;
    }

    sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
           &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1, &steal1);

    fclose(file);

    // Initialize attributes
    s = pthread_attr_init(&attr1);
    if (s != 0) handle_error_en(s, "pthread_attr_init attr1");
    s = pthread_attr_init(&attr2);
    if (s != 0) handle_error_en(s, "pthread_attr_init attr2");
    s = pthread_attr_init(&attr3);
    if (s != 0) handle_error_en(s, "pthread_attr_init attr3");
 
    // Set CPU affinity (optional)
    s = pthread_attr_setaffinity_np(&attr1, sizeof(cpu_set_t), &cpuset);
    if (s != 0) handle_error_en(s, "pthread_attr_setaffinity_np attr1");
    s = pthread_attr_setaffinity_np(&attr2, sizeof(cpu_set_t), &cpuset);
    if (s != 0) handle_error_en(s, "pthread_attr_setaffinity_np attr2");
    s = pthread_attr_setaffinity_np(&attr3, sizeof(cpu_set_t), &cpuset);
    if (s != 0) handle_error_en(s, "pthread_attr_setaffinity_np attr3");


    // Set scheduling policies and priorities
    // Thread 1: SCHED_RR, priority 10
    s = pthread_attr_setschedpolicy(&attr1, SCHED_RR);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedpolicy attr1");
    param1.sched_priority = 10;
    s = pthread_attr_setschedparam(&attr1, &param1);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedparam attr1");
    s = pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) handle_error_en(s, "pthread_attr_setinheritsched attr1");
    // Thread 2: SCHED_RR, priority 10
    s = pthread_attr_setschedpolicy(&attr2, SCHED_RR);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedpolicy attr2");
    param2.sched_priority = 10;
    s = pthread_attr_setschedparam(&attr2, &param2);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedparam attr2");
    s = pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) handle_error_en(s, "pthread_attr_setinheritsched attr2");

    // Thread 3: SCHED_RR, priority 10
    s = pthread_attr_setschedpolicy(&attr3, SCHED_RR);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedpolicy attr3");

    param3.sched_priority = 10;
    s = pthread_attr_setschedparam(&attr3, &param3);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedparam attr3");
    s = pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) handle_error_en(s, "pthread_attr_setinheritsched attr3");

    // Create threads
    thread_data[0].release_time = get_time_ms(); // Release Time for thread 1
    s = pthread_create(&thread_1, &attr1, thread1, NULL);
    if (s != 0) handle_error_en(s, "pthread_create thread_1");
    thread_data[1].release_time = get_time_ms(); // Release Time for thread 2
    s = pthread_create(&thread_2, &attr2, thread2, NULL);
    if (s != 0) handle_error_en(s, "pthread_create thread_2");
    thread_data[2].release_time = get_time_ms(); // Release Time for thread 2
    s = pthread_create(&thread_3, &attr3, thread3, NULL);
    if (s != 0) handle_error_en(s, "pthread_create thread_3");

    // Join threads
    pthread_join(thread_1, NULL);
    pthread_join(thread_2, NULL);
    pthread_join(thread_3, NULL);

    file = fopen("/proc/stat", "r");
    if (!file) return -1;

    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return -1;
    }

    sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
           &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2, &steal2);

    fclose(file);

    // Calculate deltas
    unsigned long user_d = user2 - user1;
    unsigned long nice_d = nice2 - nice1;
    unsigned long system_d = system2 - system1;
    unsigned long idle_d = idle2 - idle1;
    unsigned long iowait_d = iowait2 - iowait1;
    unsigned long irq_d = irq2 - irq1;
    unsigned long softirq_d = softirq2 - softirq1;
    unsigned long steal_d = steal2 - steal1;

    // Calculate total and idle time
    unsigned long total_d = user_d + nice_d + system_d + idle_d + iowait_d + irq_d + softirq_d + steal_d;
    unsigned long idle_total_d = idle_d + iowait_d;

    double cpu_load = 100.0 * (1.0 - (double)idle_total_d / total_d);
    float end_process_time = get_time_ms();

    double avg_total_execution_time, avg_cpu_execution_time, avg_release_time, avg_start_time, avg_finish_time, avg_wait_time, avg_response_time, avg_turnaround_time, avg_cpu_useful_work, avg_cpu_utilization, avg_memory_consumption;

    for(int i = 0 ; i < 3; i++){
        thread_data[i].turnaround_time = thread_data[i].finish_time - thread_data[i].release_time;                                                  // Turnaround Time
        thread_data[i].wait_time = thread_data[i].response_time + (thread_data[i].total_execution_time - thread_data[i].cpu_execution_time);        // Waiting Time
        thread_data[i].response_time = thread_data[i].start_time - thread_data[i].release_time;                                                     // Response Time
        thread_data[i].cpu_useful_work = thread_data[i].cpu_execution_time / (thread_data[i].cpu_execution_time + thread_data[i].wait_time);
        thread_data[i].cpu_utilization = (thread_data[i].cpu_execution_time / thread_data[i].total_execution_time) * 100;

        avg_total_execution_time += thread_data[i].total_execution_time;
        avg_cpu_execution_time += thread_data[i].cpu_execution_time;
        avg_release_time += thread_data[i].release_time;
        avg_start_time += thread_data[i].start_time;
        avg_finish_time += thread_data[i].finish_time;
        avg_wait_time += thread_data[i].wait_time;
        avg_response_time += thread_data[i].response_time;
        avg_turnaround_time += thread_data[i].turnaround_time;
        avg_turnaround_time += thread_data[i].turnaround_time;
        avg_cpu_useful_work += thread_data[i].cpu_useful_work;
        avg_cpu_utilization += thread_data[i].cpu_utilization;
        avg_memory_consumption += thread_data[i].memory_consumption;
    }

    avg_total_execution_time /= 3;
    avg_cpu_execution_time /= 3;
    avg_release_time /= 3;
    avg_start_time /= 3;
    avg_finish_time /= 3;
    avg_wait_time /= 3;
    avg_response_time /= 3;
    avg_turnaround_time /= 3;
    avg_turnaround_time /= 3;
    avg_cpu_useful_work /= 3;
    avg_cpu_utilization /= 3;
    avg_memory_consumption /= 3;

    printf("Total Process Execution Time: %.2f ms\n", end_process_time - thread_data[0].release_time);
    printf("System CPU Load: %.6f%%\n", cpu_load);

    printf("\nThread Execution Times:\n");
    for (int i = 0; i < 3; i++) {
        printf("Thread %d:\n", i + 1);
        printf("  Total Execution Time: %.2f ms\n", thread_data[i].total_execution_time);
        printf("  CPU Execution Time: %.2f ms\n", thread_data[i].cpu_execution_time);
        printf("  Release Time: %.2f ms\n", thread_data[i].release_time);
        printf("  Start Time: %.2f ms\n", thread_data[i].start_time);
        printf("  Finish Time: %.2f ms\n", thread_data[i].finish_time);
        printf("  Wait Time: %.2f ms\n", thread_data[i].wait_time);
        printf("  Response Time: %.2f ms\n", thread_data[i].response_time);
        printf("  Turnaround Time: %.2f ms\n", thread_data[i].turnaround_time);
        printf("  CPU Useful Work: %.2f ms\n", thread_data[i].cpu_useful_work);
        printf("  CPU Utilization: %.2f%%\n", thread_data[i].cpu_utilization);
        printf("  Memory Consumption: %.2f KB\n \n", thread_data[i].memory_consumption);
    }

    printf("\nAverages:\n");
    printf("  Average Total Execution Time: %.2f ms\n", avg_total_execution_time);
    printf("  Average CPU Execution Time: %.2f ms\n", avg_cpu_execution_time);
    printf("  Average Release Time: %.2f ms\n", avg_release_time);
    printf("  Average Start Time: %.2f ms\n", avg_start_time);
    printf("  Average Finish Time: %.2f ms\n", avg_finish_time);
    printf("  Average Wait Time: %.2f ms\n", avg_wait_time);
    printf("  Average Response Time: %.2f ms\n", avg_response_time);
    printf("  Average Turnaround Time: %.2f ms\n", avg_turnaround_time);
    printf("  Average CPU Useful Work: %.2f ms\n", avg_cpu_useful_work);
    printf("  Average CPU Utilization: %.2f%%\n", avg_cpu_utilization);
    printf("  Average Memory Consumption: %.2f KB\n\n", avg_memory_consumption);

    // Clean up attributes
    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);
    pthread_attr_destroy(&attr3);

    return 0;
}