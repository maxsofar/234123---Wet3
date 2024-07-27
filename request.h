#ifndef __REQUEST_H__

typedef struct {
    int id;
    int total_req;
    int stat_req;
    int dynm_req;
} thread_stats_t;

void requestHandle(int fd, struct timeval arrival, struct timeval dispatch, thread_stats_t *t_stats);

#endif