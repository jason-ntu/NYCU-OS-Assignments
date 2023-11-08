#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sched.h>

pthread_barrier_t barrier;
int num_threads = 0;
float time_wait;
int target_cpu = 0;

void *thread_func(void *arg)
{
    /* 1. Wait until all threads are ready */
    int* thread_num = (int*)arg;
    pthread_barrier_wait(&barrier);

    // Check the CPU affinity for this thread
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset); // Initialize the CPU set
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    assert(CPU_ISSET(target_cpu, &cpuset));

    /* 2. Do the task */ 
    time_t start_time;
    time_t current_time;
    for (int i = 0; i < 3; i++) {
        printf("Thread %d is running\n", *thread_num);
        /* Busy for <time_wait> seconds */
        time(&start_time);
        do {
            time(&current_time);
        } while(current_time - start_time < time_wait);   
    }
    /* 3. Exit the function  */
    return 0;
}

int *parse_policies(char *str, int len) {
    int *result = (int *)malloc(len * sizeof(int));
    char *token = strtok(str, ",");

    for (int i = 0; token != NULL && i < len; i++) {
        if (strcmp(token, "NORMAL") == 0) {
            result[i] = SCHED_OTHER;
        } else if (strcmp(token, "FIFO") == 0) {
            result[i] = SCHED_FIFO;
        } else {
            fprintf(stderr, "Invalid scheduling policy: %s\n", token);
            break;
        }
        token = strtok(NULL, ",");
    }
    return result;
}

int *parse_priorities(char *str, int len) {
    int *result = (int *)malloc(len * sizeof(int));
    char *token = strtok(str, ",");

    for (int i = 0; token != NULL && i < len; i++) {
        result[i] = atoi(token);
        token = strtok(NULL, ",");
    }

    return result;
}

int main(int argc, char *argv[]) {

    /* 1. Parse program arguments */
    int opt;
    char* endptr;
    char *policy_arg = NULL;
    char *priority_arg = NULL;

    while ((opt = getopt(argc, argv, "n:t:s:p:")) != -1) {
        switch (opt) {
            case 'n':
                num_threads = atoi(optarg);
                break;
            case 't':
                time_wait = strtof(optarg, &endptr);
                break;
            case 's':
                policy_arg = optarg;
                break;
            case 'p':
                priority_arg = optarg;
                break;
            default:
                break;
        }
    }

    assert(num_threads);
    assert(time_wait);
    assert(policy_arg);
    assert(priority_arg);
    
    int *policies = parse_policies(policy_arg, num_threads);
    int *priorities = parse_priorities(priority_arg, num_threads);

    // for (int i = 0; i < num_threads; i++) {
    //     printf("policies[%d] = %i\n", i, policies[i]);
    //     printf("priorities[%d] = %d\n", i, priorities[i]);
    // }

    /* 2. Create <num_threads> worker threads */
    pthread_t thread_ids[num_threads];
    pthread_attr_t attrs[num_threads];
    assert(pthread_barrier_init(&barrier, NULL, num_threads) == 0);
    
    /* 3. Set CPU affinity */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(target_cpu, &cpuset);
    assert(sched_setaffinity(getpid(), sizeof(cpuset), &cpuset) == 0);
    
    for (int i = 0; i < num_threads; i++) {
        /* 4. Set the attributes to each thread */
        pthread_attr_init(&attrs[i]);
        assert(pthread_attr_setinheritsched(&attrs[i], PTHREAD_EXPLICIT_SCHED) == 0);
        if (policies[i] == SCHED_FIFO) {
            assert(pthread_attr_setschedpolicy(&attrs[i], SCHED_FIFO) == 0);
            struct sched_param param;
            param.sched_priority = priorities[i];
            assert(pthread_attr_setschedparam(&attrs[i], &param) == 0);
        }
    }

    /* 5. Start all threads at once */
    for (int i = 0; i < num_threads; i++) {
        int *thread_num = (int*) malloc(sizeof(int));
        *thread_num = i;
        assert(pthread_create(&thread_ids[i], &attrs[i], thread_func, thread_num) == 0);
    }

    /* 6. Wait for all threads to finish  */
    for (int i = 0; i < num_threads; i++) {
        assert(pthread_join(thread_ids[i], NULL) == 0);
        pthread_attr_destroy(&attrs[i]);
    }

    assert(pthread_barrier_destroy(&barrier) == 0);
    
    return 0;
}
