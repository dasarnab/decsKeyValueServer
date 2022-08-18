struct config
{
    char ipv4_addr[16];
    char port[6];
    int backlog;
    int threadpoolsize;
    int maxevents;
};