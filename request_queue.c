//
// Created by Max on 26/07/2024.
//

#include "request_queue.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "segel.h"

void init(request_queue_t *queue, int capacity) {
    queue->buffer = (int *)malloc(capacity * sizeof(int));
    queue->arrival_times = (struct timeval *)malloc(capacity * sizeof(struct timeval));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
}

int enqueue(request_queue_t *queue, int connfd, char *schedalg) {
    if (queue->size == queue->capacity) {
        return -1; // Queue is full
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->buffer[queue->rear] = connfd;
    gettimeofday(&queue->arrival_times[queue->rear], NULL);
    queue->size++;
    return 0; // Successfully enqueued
}

int dequeue(request_queue_t *queue, struct timeval *arrival_time) {
    if (queue->size == 0) {
        return -1; // Queue is empty
    }
    int request = queue->buffer[queue->front];
    *arrival_time = queue->arrival_times[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return request;
}


