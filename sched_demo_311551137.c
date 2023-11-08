#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

pthread_barrier_t barrier;
int num_threads = 0;
float time_wait;

typedef struct {
    pthread_t thread_id;
    int thread_num;
    int sched_policy;
    int sched_priority;
} thread_info_t;

void *thread_func(void *arg)
{
    /* 1. Wait until all threads are ready */
    thread_info_t* thread_info = (thread_info_t*)arg;
    pthread_barrier_wait(&barrier);

    /* 2. Do the task */ 
    time_t start_time;
    time_t current_time;
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", thread_info->thread_num);
        /* Busy for <time_wait> seconds */
        time(&start_time);
        do {
            time(&current_time);
        } while(current_time - start_time < time_wait);
        
    }
    /* 3. Exit the function  */
    return 0;
}

int main(int argc, char *argv[]) {

    /* 1. Parse program arguments */
    int opt;
    char* endptr;
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

    assert(num_threads);
    assert(time_wait);
    assert(policies);
    assert(priorities);

    /* 2. Create <num_threads> worker threads */
    thread_info_t thread_infos[num_threads];
    pthread_barrier_init(&barrier, NULL, num_threads);
    
    /* 3. Set CPU affinity */

    for (int i = 0; i < num_threads; i++) {
        /* 4. Set the attributes to each thread */
    }

    /* 5. Start all threads at once */
    for (int i = 0; i < num_threads; i++) {
        thread_infos[i].thread_num = i;
        pthread_create(&(thread_infos[i].thread_id), NULL, thread_func, &thread_infos[i]);
    }

    /* 6. Wait for all threads to finish  */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(thread_infos[i].thread_id, NULL);
    }

    pthread_barrier_destroy(&barrier);

    return 0;
}
