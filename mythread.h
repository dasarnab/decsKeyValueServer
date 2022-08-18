#include <pthread.h>
struct context
{
    int activeClints;
    int epollfd;
    int maxevents;
    struct epoll_event *ev;
    struct epoll_event *ep_events;
};
struct mythread
{
    pthread_t tid;
    struct context *context;
    // mutex
    // condition variable
};