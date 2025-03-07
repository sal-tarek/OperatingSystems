#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// release time
// start time 
// finish time
// wait time = turnaround - execution
// response time = start - release
// turnaround time = finish - release
// execution time = finish - start

// Error handling macro
#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

// Function to get the current time in milliseconds
double get_time_ms(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6; // Convert to milliseconds
}

// Structure to hold timing data
typedef struct {
    double release_time;
    double start_time;
    double finish_time;
    double wait_time;
    double response_time;
    double turnaround_time;
    double execution_time; // Total execution time (wall-clock)
    double cpu_time; // CPU execution time
} ThreadMetrics;

ThreadMetrics thread_data[3];


// Thread 1 - Nour, Mai, Yasmeen
void *thread1(void *arg) {
    thread_data[0].start_time = get_time_ms();  // Start time
    char char1, char2;

    printf("Thread 1 input: Please enter two characters =(\n");
    printf("First character: \n");
    scanf(" %c", &char1);
    printf("Second character: \n");
    scanf(" %c", &char2);

    for (char i = char1; i <= char2; i++) 
        printf("%c", i);

    printf("\n");
    thread_data[0].finish_time = get_time_ms();  // Finish time
    pthread_exit(NULL);
    return NULL;
}

//Thread 2 - Lama, Habiba
void *thread2(void *arg) {
    thread_data[1].start_time = get_time_ms();  // Start time
    pthread_t threadID = pthread_self();
    printf("First Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Second Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Third Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Bye!\n");
    thread_data[1].finish_time = get_time_ms();  // Finish time

    pthread_exit(NULL);
    return NULL;
}

// Thread 3 - Salma, Layla (SCHED_FIFO)
void *thread3(void *arg) {
    thread_data[2].start_time = get_time_ms();  // Start time
    int num1, num2;

    printf("Thread 3 input: Please enter two numbers =)\n");
    printf("First number: \n");
    scanf("%d", &num1);
    printf("Second number: \n");
    scanf("%d", &num2);

    int sum = 0, prod = 1;
    float avg = 0; // Changed to float to handle division accurately
    for (int i = num1; i <= num2; i++) {
        sum += i;
        prod *= i;
    }
    avg = (float)sum / (num2 - num1 + 1);
    printf("Sum: %d\n", sum);
    printf("Average: %.2f\n", avg);
    printf("Product: %d\n", prod);
    
    thread_data[2].finish_time = get_time_ms();  // Finish time
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
    // Thread 1: SCHED_FIFO, priority 10
    s = pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedpolicy attr1");
    param1.sched_priority = 10;
    s = pthread_attr_setschedparam(&attr1, &param1);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedparam attr1");
    s = pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) handle_error_en(s, "pthread_attr_setinheritsched attr1");

    // Thread 2: SCHED_FIFO, priority 10
    s = pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedpolicy attr2");
    param2.sched_priority = 10;
    s = pthread_attr_setschedparam(&attr2, &param2);
    if (s != 0) handle_error_en(s, "pthread_attr_setschedparam attr2");
    s = pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
    if (s != 0) handle_error_en(s, "pthread_attr_setinheritsched attr2");

    // Thread 3: SCHED_FIFO, priority 10
    s = pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
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
    pthread_join(thread_1, NULL);
    printf("Thread 1 has finished execution.\n \n");
    thread_data[1].release_time = get_time_ms(); // Release Time for thread 2
    s = pthread_create(&thread_2, &attr2, thread2, NULL);
    if (s != 0) handle_error_en(s, "pthread_create thread_2");
    pthread_join(thread_2, NULL);
    printf("Thread 2 has finished execution.\n \n");
    thread_data[2].release_time = get_time_ms(); // Release Time for thread 2
    s = pthread_create(&thread_3, &attr3, thread3, NULL);
    if (s != 0) handle_error_en(s, "pthread_create thread_3");
    pthread_join(thread_3, NULL);
    printf("Thread 3 has finished execution.\n \n");


    for(int i = 0 ; i < 3; i++){
        printf("release time for thread %d: %f\n", i + 1, thread_data[i].release_time);
        printf("start time for thread %d: %f\n", i + 1, thread_data[i].start_time);
        printf("finish time for thread %d: %f\n", i + 1, thread_data[i].finish_time);

        // Response Time
        thread_data[i].response_time = thread_data[i].start_time - thread_data[i].release_time;
        printf("response time for thread %d: %f\n", i + 1, thread_data[i].response_time);
        // Turnaround Time
        thread_data[i].turnaround_time = thread_data[i].finish_time - thread_data[i].release_time;
        printf("turnaround time for thread %d: %f\n", i + 1, thread_data[i].turnaround_time);
        // Execution Time
        thread_data[i].execution_time = thread_data[i].finish_time - thread_data[i].start_time;
        printf("execution time for thread %d: %f\n", i + 1, thread_data[i].execution_time);
        // Waiting Time  
        thread_data[i].wait_time = thread_data[i].turnaround_time - thread_data[i].execution_time;
        printf("wait time for thread %d: %f\n\n", i + 1, thread_data[i].wait_time);
        
    }

    // Clean up attributes
    pthread_attr_destroy(&attr1);
    pthread_attr_destroy(&attr2);
    pthread_attr_destroy(&attr3);

    return 0;
}