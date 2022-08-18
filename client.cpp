#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;

int main()
{
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo("127.0.0.1", "1234", &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }

    connect(sock_fd, result->ai_addr, result->ai_addrlen);

    char buffer[100];
    while (fgets(buffer, 100, stdin) != NULL)
    {
        printf("SENDING: %s", buffer);
        printf("===\n");
        write(sock_fd, buffer, strlen(buffer));

        char resp[1000];
        int len = read(sock_fd, resp, 999);
        resp[len] = '\0';
        printf("%s\n", resp);
    }
    return 0;
}