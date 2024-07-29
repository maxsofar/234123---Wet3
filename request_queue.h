//
// Created by Max on 26/07/2024.
//

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <pthread.h>

typedef struct {
    int *buffer;
    struct timeval *arrival_times;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    pthread_cond_t empty;
} request_queue_t;

void init(request_queue_t *queue, int capacity);
void enqueue(request_queue_t *queue, int connfd, struct timeval arrival_time);
int dequeue(request_queue_t *queue, struct timeval *arrival_time);

#endif // REQUEST_QUEUE_H
