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
} request_queue_t;

void init(request_queue_t *queue, int capacity);
int enqueue(request_queue_t *queue, int connfd, char *schedalg);
int dequeue(request_queue_t *queue, struct timeval *arrival_time);

#endif // REQUEST_QUEUE_H


