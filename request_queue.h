//
// Created by Max on 26/07/2024.
//

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include <pthread.h>

typedef struct {
    int *buffer;
    int capacity;
    int size;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} request_queue_t;

void init(request_queue_t *queue, int capacity);
void destroy(request_queue_t *queue);
void enqueue(request_queue_t *queue, int request);
int dequeue(request_queue_t *queue);
int dequeue_last(request_queue_t *queue);

#endif // REQUEST_QUEUE_H


