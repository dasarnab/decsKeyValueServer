#include <iostream>
#include <pthread.h>
#include "mythread.h"
using namespace std;
void *serverthread(void *);

int init_thread(struct mythread *th, int maxevents)
{
    th->context = new struct context;
    // if ((th->context->epollfd = epoll_create(1)) == -1)
    // {
    //     perror("epoll create");
    //     exit(1);
    // }
    th->context->activeClints = 0;
    th->context->epollfd = 5;
    th->context->maxevents = maxevents;
    // th->context->ev = new struct epoll_event;
    // th->context->ep_events = new struct epoll_event[maxevents];
    return 0;
}
void *serverthread(void *arg)
{
    struct mythread thisthread = *((struct mythread *)arg);
    printf("hello from %u\nprinting my content:", thisthread.tid);
    printf("epollfd : %d\tmaxevetns : %d\n", thisthread.context->epollfd, thisthread.context->maxevents);
    return NULL;
}