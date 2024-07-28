#include "segel.h"
#include "request.h"
#include "request_queue.h"

//
// server.c: Multi-threaded web server
//
// To run:
//  ./server <port> <threads> <queue_size> <schedalg>
//

void getargs(int *port, int *threads, int *queue_size, char **schedalg, int argc, char *argv[])
{
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

pthread_mutex_t queue_mutex;
pthread_cond_t queue_not_full;
pthread_cond_t queue_not_empty;

void *worker_thread(void *arg) {
    thread_stats_t *t_stats = (thread_stats_t *)arg;
    while (1) {
        struct timeval arrival_time, dispatch_time, current_time;
        gettimeofday(&arrival_time, NULL);

        pthread_mutex_lock(&queue_mutex);
        while (request_queue.size == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_mutex);
        }
        int connfd = dequeue(&request_queue, &arrival_time);
        pthread_cond_signal(&queue_not_full);
        pthread_mutex_unlock(&queue_mutex);

        // Calculate dispatch interval
        gettimeofday(&current_time, NULL);
        timersub(&current_time, &arrival_time, &dispatch_time);

        // Update thread statistics
        t_stats->total_req++;

        // Format dispatch time as a string
        char dispatch_time_str[64];
        snprintf(dispatch_time_str, sizeof(dispatch_time_str), "%ld.%06ld", dispatch_time.tv_sec, dispatch_time.tv_usec);

        requestHandle(connfd, arrival_time, dispatch_time, t_stats);
        Close(connfd);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen;
    int threads, queue_size;
    char *schedalg;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &queue_size, &schedalg, argc, argv);

    init(&request_queue, queue_size);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_full, NULL);
    pthread_cond_init(&queue_not_empty, NULL);

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
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        pthread_mutex_lock(&queue_mutex);
        while (request_queue.size == request_queue.capacity) {
            pthread_cond_wait(&queue_not_full, &queue_mutex);
        }
        if (enqueue(&request_queue, connfd, schedalg) == -1) {
            Close(connfd);
        }
        pthread_cond_signal(&queue_not_empty);
        pthread_mutex_unlock(&queue_mutex);
    }
}





