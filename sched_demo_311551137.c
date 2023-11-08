#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

pthread_barrier_t barrier;

void *thread_func(void *arg)
{
    /* 1. Wait until all threads are ready */
    printf("Thread is ready!\n");

    int result = pthread_barrier_wait(&barrier);

    if (result == PTHREAD_BARRIER_SERIAL_THREAD) {
        printf("All threads have reached the barrier.\n");
    }

    /* 2. Do the task */ 
    for (int i = 0; i < 3; i++) {
        // printf("Thread %d is running\n", arg[1]);
        /* Busy for <time_wait> seconds */
    }
    /* 3. Exit the function  */
}

int main(int argc, char *argv[]) {

    /* 1. Parse program arguments */

    int opt;
    char* endptr;
    int num_threads = 0;
    float time_wait;
    char *policies = NULL;
    int priorities = 0;

    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 't':
                time_wait = strtof(optarg, &endptr);
                break;
            case 's':
                policies = optarg;
                break;
            case 'p':
                priorities = atoi(optarg);
                break;
            default:
                break;
        }
    }

    // printf("num_threads = %d\n", num_threads);
    // printf("time_wait = %.1f\n", time_wait);
    // printf("policies = %s\n", policies);
    // printf("priorities = %d\n", priorities);

    assert(num_threads);
    assert(time_wait);
    assert(policies);
    assert(priorities);

    /* 2. Create <num_threads> worker threads */
    pthread_t thread_ids[num_threads];
    int thread_args[num_threads];
    pthread_barrier_init(&barrier, NULL, num_threads);
    
    /* 3. Set CPU affinity */

    for (int i = 0; i < num_threads; i++) {
        /* 4. Set the attributes to each thread */
    }

    /* 5. Start all threads at once */
    for (int i = 0; i < num_threads; i++) {
        thread_args[i] = i;
        pthread_create(&thread_ids[i], NULL, thread_func, &thread_args[i]);
    }

    /* 6. Wait for all threads to finish  */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}
