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
    queue->capacity = capacity;
    queue->size = 0;
    queue->front = 0;
    queue->rear = -1;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
}

void destroy(request_queue_t *queue) {
    free(queue->buffer);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_full);
    pthread_cond_destroy(&queue->not_empty);
}

void enqueue(request_queue_t *queue, int request) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == queue->capacity) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->buffer[queue->rear] = request;
    queue->size++;
    pthread_cond_signal(&queue->not_empty);
    pthread_mutex_unlock(&queue->mutex);
}

int dequeue(request_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    int request = queue->buffer[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return request;
}

int dequeue_last(request_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->size == 0) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    int request = queue->buffer[queue->rear];
    queue->rear = (queue->rear - 1 + queue->capacity) % queue->capacity;
    queue->size--;
    pthread_cond_signal(&queue->not_full);
    pthread_mutex_unlock(&queue->mutex);
    return request;
}
