#include <iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "wrap.h"

using namespace std;


#define MAX_LEN 256
#define LISTEN_PORT 8888
#define MAX_CLI_NUM 100

bool is_running = true;

void signalstop(int signum)
{
    is_running = false;
    printf("catch signal!\n");
}


int main()
{
    //signal(SIGINT, signalstop);

    char str[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16

    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(LISTEN_PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(listenfd, MAX_CLI_NUM);

    char buf[MAX_LEN];
    struct sockaddr_in cliaddr;
    printf("Accepting connections ...\n");
    while (1)
    {
        socklen_t cliaddr_len = sizeof(cliaddr);
        int connfd = Accept(listenfd,
                     (struct sockaddr *)&cliaddr, &cliaddr_len);
        while (1)
        {
            int n = Read(connfd, buf, MAX_LEN);
            if (n == 0)
            {
                printf("the other side has been closed.\n");
                break;
            }
            printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                ntohs(cliaddr.sin_port));

            int i = 0;
            for (i = 0; i < n; i++)
                buf[i] = toupper(buf[i]);

            Write(connfd, buf, n);
        }
        Close(connfd);
    }

    return 0;
}
