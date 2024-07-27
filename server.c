#include "segel.h"
#include "request.h"
#include "request_queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
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

void *worker_thread(void *arg) {
    while (1) {
        int connfd = dequeue(&request_queue);
        requestHandle(connfd);
        Close(connfd);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    int threads, queue_size;
    char *schedalg;
    struct sockaddr_in clientaddr;

    getargs(&port, &threads, &queue_size, &schedalg, argc, argv);

    // Initialize request queue
    init(&request_queue, queue_size);

    // Create worker threads
    pthread_t *thread_pool = malloc(threads * sizeof(pthread_t));
    for (int i = 0; i < threads; i++) {
        pthread_create(&thread_pool[i], NULL, worker_thread, NULL);
    }

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        // Enqueue the connection descriptor
        enqueue(&request_queue, connfd);
    }

    // Cleanup
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_pool[i], NULL);
    }
    free(thread_pool);
    destroy(&request_queue);

    return 0;

}





