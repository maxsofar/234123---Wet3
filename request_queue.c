//
// Created by Max on 26/07/2024.
//

#include "request_queue.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void init(request_queue_t *queue, int capacity) {
    queue->buffer = (int *)malloc(capacity * sizeof(int));
    queue->arrival_times = (struct timeval *)malloc(capacity * sizeof(struct timeval));
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->empty, NULL);
}

void enqueue(request_queue_t *queue, int connfd, struct timeval arrival_time) {
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->buffer[queue->rear] = connfd;
    queue->arrival_times[queue->rear] = arrival_time;
    queue->size++;
    pthread_cond_signal(&queue->not_empty);

}

int dequeue(request_queue_t *queue, struct timeval *arrival_time) {
    int request = queue->buffer[queue->front];
    *arrival_time = queue->arrival_times[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;

    pthread_cond_signal(&queue->not_full);

    return request;
}



