#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#define NUM_THREADS 3


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


//FCFS
//void* thread_function(void* arg){
//    int thread_num = *(int*)arg;
//    pthread_t thread_id = pthread_self();
//    printf("Thread %d: Thread ID is %lu\n", thread_num, thread_id)
//    printf("Thread %d: Starting execution..\n",thread_num);
//}
int main() {

    //pthread_t threads[3];
    //int thread_nums[3] = {1, 2, 3};

    //for(int i=0; i<3, i++){
      //pthread_create(&threads[i], NULL, thread_function, &thread_nums[i]);
      //pthread_join(threads[i], NULL);
    }
        pthread_t threads[NUM_THREADS];
        pthread_attr_t attr;
        struct sched_param param;
        
        // Initialize thread attributes
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // Set FCFS (FIFO) scheduling
        param.sched_priority = 1; // Assign priority
        pthread_attr_setschedparam(&attr, &param);
    
        // Create threads
        pthread_create(&threads[0], &attr, thread1, NULL);
        pthread_create(&threads[1], &attr, thread2, NULL);
        pthread_create(&threads[2], &attr, thread3, NULL);
    
        // Wait for threads to complete
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }
    
        pthread_attr_destroy(&attr);
        printf("All threads executed successfully in FCFS order.\n");
        return 0;
    }

