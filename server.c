#include "segel.h"
#include "request.h"
#include "request_queue.h"

//
// server.c: Multi-threaded web server
//
// To run:
//  ./server <port> <threads> <queue_size> <schedalg>
//

typedef enum {
    SCHED_BLOCK,
    SCHED_DT,
    SCHED_DH,
    SCHED_BF,
    SCHED_RD
} schedalg_t;

schedalg_t parse_schedalg(char *schedalg) {
    if (strcmp(schedalg, "block") == 0) return SCHED_BLOCK;
    if (strcmp(schedalg, "dt") == 0) return SCHED_DT;
    if (strcmp(schedalg, "dh") == 0) return SCHED_DH;
    if (strcmp(schedalg, "bf") == 0) return SCHED_BF;
    if (strcmp(schedalg, "random") == 0) return SCHED_RD;
    fprintf(stderr, "Unknown scheduling algorithm: %s\n", schedalg);
    exit(1);
}

void getargs(int *port, int *threads, int *queue_size, char **schedalg, int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port> <threads> <queue_size> <schedalg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];
}

request_queue_t request_queue;

int active_threads = 0;

void *worker_thread(void *arg) {
    thread_stats_t *t_stats = (thread_stats_t *)arg;
    while (1) {
        struct timeval arrival_time, dispatch_time, current_time;

        pthread_mutex_lock(&request_queue.mutex);
        while (request_queue.size == 0) {
            pthread_cond_wait(&request_queue.not_empty, &request_queue.mutex);
        }
        int connfd = dequeue(&request_queue, &arrival_time);
        active_threads++;
        pthread_mutex_unlock(&request_queue.mutex);

        // Calculate dispatch interval
        gettimeofday(&current_time, NULL);
        timersub(&current_time, &arrival_time, &dispatch_time);

        // Update thread statistics
        t_stats->total_req++;

        requestHandle(connfd, arrival_time, dispatch_time, t_stats);
        Close(connfd);

        pthread_mutex_lock(&request_queue.mutex);
        active_threads--;
        if (request_queue.size == 0 && active_threads == 0) {
            pthread_cond_signal(&request_queue.empty);
        }
        pthread_mutex_unlock(&request_queue.mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen, threads, queue_size;
    struct sockaddr_in clientaddr;
    char *schedalg_str;
    schedalg_t schedalg;

    getargs(&port, &threads, &queue_size, &schedalg_str, argc, argv);
    schedalg = parse_schedalg(schedalg_str);

    init(&request_queue, queue_size);

    pthread_t *thread_pool = malloc(threads * sizeof(pthread_t));
    thread_stats_t *thread_stats = malloc(threads * sizeof(thread_stats_t));
    for (int i = 0; i < threads; i++) {
        thread_stats[i].id = i;
        thread_stats[i].total_req = 0;
        thread_stats[i].stat_req = 0;
        thread_stats[i].dynm_req = 0;
        pthread_create(&thread_pool[i], NULL, worker_thread, &thread_stats[i]);
    }

    listenfd = Open_listenfd(port);
    srand(time(NULL)); // Initialize random seed for drop_random


    while (1) {
        struct timeval arrival_time;
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);

        gettimeofday(&arrival_time, NULL);

        pthread_mutex_lock(&request_queue.mutex);
        if (request_queue.size == request_queue.capacity) {
            if (schedalg == SCHED_BF) {
                while (request_queue.size > 0) {
                    pthread_cond_wait(&request_queue.empty, &request_queue.mutex);
                }
                pthread_mutex_unlock(&request_queue.mutex);
                Close(connfd);
                continue;
            } else if (schedalg == SCHED_BLOCK) {
                while (request_queue.size == request_queue.capacity) {
                    pthread_cond_wait(&request_queue.not_full, &request_queue.mutex);
                }

            } else if (schedalg == SCHED_DT) {
                pthread_mutex_unlock(&request_queue.mutex);
                Close(connfd);
                continue;

            } else if (schedalg == SCHED_DH) {
                // Drop the oldest request
                struct timeval ignored_time;
                dequeue(&request_queue, &ignored_time);

            } else if (schedalg == SCHED_RD) {
                // Drop 50% of the requests randomly
                int drop_count = request_queue.size / 2;
                for (int i = 0; i < drop_count; i++) {
                    int index = rand() % request_queue.size;
                    for (int j = index; j < request_queue.size - 1; j++) {
                        request_queue.buffer[j] = request_queue.buffer[j + 1];
                        request_queue.arrival_times[j] = request_queue.arrival_times[j + 1];
                    }
                    request_queue.size--;
                }
            }
        }
        enqueue(&request_queue, connfd, arrival_time);
        pthread_mutex_unlock(&request_queue.mutex);
    }
}





