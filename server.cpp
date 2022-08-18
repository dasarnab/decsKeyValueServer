#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "mythread.h"
using namespace std;

struct mythread *setUpThreads(struct config *);
extern void *serverthread(void *);
extern int init_thread(struct mythread *, int);
int parse_config(const char *filename, struct config *conf)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("Couldn't open file at %s\n", filename);
        return -1;
    }
    if (fscanf(f, "%s\n", (conf->ipv4_addr)) != 1)
    {
        printf("wrong input file format\n");
        return -1;
    };
    if (fscanf(f, "%s\n", (conf->port)) != 1)
    {
        printf("wrong input file format\n");
        return -1;
    };
    if (fscanf(f, "%d\n", &(conf->backlog)) != 1)
    {
        printf("wrong input file format\n");
        return -1;
    };
    if (fscanf(f, "%d\n", &(conf->threadpoolsize)) != 1)
    {
        printf("wrong input file format\n");
        return -1;
    };
    if (fscanf(f, "%d\n", &(conf->maxevents)) != 1)
    {
        printf("wrong input file format\n");
        return -1;
    };
    printf("IPV4 : %s\nport: %s\nthreadpoolsize : %d\n", conf->ipv4_addr, conf->port, conf->threadpoolsize);
    return 0;
}

int main()
{
    struct config *conf = new struct config;
    const char *inputFileName = "./config.txt";
    if (parse_config(inputFileName, conf))
    {
        perror("parsing error\n");
    }

    /* -------------------------Server Setup-----------------------*/
    struct addrinfo hints, *result, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(conf->ipv4_addr, conf->port, &hints, &result);

    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    int listener = -1;
    int optval = 1;
    for (p = result; p != NULL; p = p->ai_next)
    {
        if ((listener = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(listener);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(result);
    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(listener, conf->backlog) != 0)
    {
        perror("listen()");
        exit(1);
    }
    printf("Waiting for connection...\n");
    /* -------------------------Server Setup-----------------------*/

    struct mythread *curr_tpool = setUpThreads(conf);
    curr_tpool[0].context->ev.events = EPOLLIN;
    curr_tpool[0].context->ev.data.fd = listener;
    if (epoll_ctl(curr_tpool[0].context->epollfd, EPOLL_CTL_ADD, listener, curr_tpool[0].context->ev) == -1)
    {
        perror("epoll_ctrl error\n");
    }
    int nfds = 0;
    while (true)
    {
        if ((nfds = epoll_wait(curr_tpool[0].context->epollfd, curr_tpool[0].context->ep_events, curr_tpool[0].context->maxevents, -1)) == -1)
        {
            perror("epoll_wait\n");
        }
        for (int i = 0; i < nfds; i++)
        {
            if ((curr_tpool[0].context->ep_events[i].events & EPOLLIN) == EPOLLIN)
            {
                if (curr_tpool[0].context->ep_events[i].data.fd == listener)
                {
                    // new connection accept
                    // 1. increment activeclient of next thread we need  mutex
                    // 2. if active_client == 1 wake it up
                    // 3. Add client_fd next thread's epollfd
                }
            }
        }
    }

    for (int i = 1; i < conf->threadpoolsize + 1; i++)
    {
        pthread_join(curr_tpool[i].tid, NULL);
    }
    return 0;
}
struct mythread *setUpThreads(struct config *conf)
{

    struct mythread *threadpool;
    threadpool = new struct mythread[conf->threadpoolsize];
    threadpool[0].tid = pthread_self();
    init_thread(&threadpool[0], 1);

    for (int i = 1; i < conf->threadpoolsize + 1; i++)
    {
        init_thread(&threadpool[i], conf->maxevents);
        pthread_create(&(threadpool[i].tid), NULL, serverthread, (void *)&threadpool[i]);
    }
    return threadpool;
}