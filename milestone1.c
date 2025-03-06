#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


//FCFS
// void* thread_function(void* arg){
//     int thread_num = *(int*)arg;
//     pthread_t thread_id = pthread_self();
    
// }

//RoundRobin


// Thread 1 - Nour, Mai, Yasmeen
void *thread1(void *arg) {
    char char1;
    char char2;
    printf("Please enter two characters =(\n");
    printf("First character: \n");
    scanf(" %c", &char1);
    printf("Second character: \n");
    scanf(" %c", &char2);

    for (char i = char1; i <= char2; i++) {
        printf("%c", i);
    }
    printf("\n");
    return NULL;
}

//Thread2 - Lama, Habiba
void *thread2(void *arg) {
    pthread_t threadID = pthread_self();
    printf("First Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Second Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Third Hi from thread %ld!\n",(unsigned long)threadID);
    printf("Bye!\n");
    pthread_exit(NULL);
    return NULL;
}

//Thread 3 - Salma, Layla
void* thread3(void *arg) {
    int num1 = 0;
    int num2 = 0;
    printf("Please enter two numbers =)\n");
    printf("First number: \n");
    scanf("%d",&num1);
    printf("Second number: \n");
    scanf("%d",&num2);
    
    int sum = 0;
    int avg = 0;
    int prod = 1;
    
    for(int i=num1; i<=num2; i++){
        sum += i;
        prod *= i;
    }
    avg = sum/(num2-num1+1);
    printf("Sum: %d\n",sum);
    printf("Average: %d\n",avg);
    printf("Product: %d\n",prod);
    return NULL;
}

int main() {
    // CPU affinity initialization
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset); 
    CPU_SET(0, &cpuset);
    pthread_attr_t attr; 
    pthread_attr_init(&attr); 
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    // Creation of threads
    pthread_t thread_1, thread_2, thread_3;

    // Thread 1
    pthread_create(&thread_1, &attr, thread1, NULL);
    pthread_join(thread_1, NULL);
    printf("Thread 1 has finished execution.\n");

    // Thread 2
    pthread_create(&thread_2, &attr, thread2, NULL);
    pthread_join(thread_2, NULL);
    printf("Thread 2 has finished execution.\n");

    // Thread 3
    pthread_create(&thread_3, &attr, thread3, NULL);
    pthread_join(thread_3,NULL);
    printf("Thread 3 has finished execution.\n");

    return 0;
}