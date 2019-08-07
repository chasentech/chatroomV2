#include <iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>

#include "wrap.h"

using namespace std;


#define MAX_LEN 256
#define LISTEN_PORT 8888
#define MAX_CLI_NUM 100

bool is_running = true;


static int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

void signalstop(int signum)
{
    is_running = false;
    printf("catch signal!\n");
    Close(listenfd);
    exit(0);
}


int main()
{
    signal(SIGINT, signalstop);

    //int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(LISTEN_PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(listenfd, MAX_CLI_NUM);

    int maxi = -1;
    int maxfd = listenfd;
    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    int i = 0;
    int client_fd[FD_SETSIZE]; //FD_SETSIZE为1024
    for (i = 0; i < FD_SETSIZE; i++)
        client_fd[i] = -1;

    char str_IP[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16
    char buf[MAX_LEN] = {0};
    printf("Accepting connections ...\n");

    while (is_running)
    {
        rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (nready < 0)
        {
            perror("select error:");
        }

        if (FD_ISSET(listenfd, &rset)) // new connect arived
        {

            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof(cliaddr);
            int connfd = Accept(listenfd,
                         (struct sockaddr *)&cliaddr, &cliaddr_len);

            printf("connect from %s at PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, str_IP, sizeof(str_IP)),
                ntohs(cliaddr.sin_port));

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client_fd[i] < 0)
                {
                    client_fd[i] = connfd; //save descriptor
                    break;
                }
            }
            if (i == FD_SETSIZE)
            {
                printf("too many clients\n");
                exit(1);
            }

            FD_SET(connfd, &allset);
            if (connfd > maxfd) maxfd = connfd; //maxfd for select
            if (i > maxi) maxi = i;     //maxi in client_fd

            printf("max fd: %d\n", maxi + 1);

            if (--nready == 0) continue;
        }

        for (i = 0; i <= maxi; i++) //already connect sockfd
        {
            int sockfd = client_fd[i];
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset))
            {
                int n = Read(sockfd, buf, MAX_LEN);
                if (n == 0)
                {
                    printf("the other side has been closed.\n");
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client_fd[i] = -1;
                } else
                {
                    printf("received buf: %s", buf);

                    //do something
                    //...

                    Write(sockfd, buf, n);
                    memset(buf, 0, n);
                }

                if (--nready == 0) break; //notice maxi and nready relationship
            }
        }
    }


    //Close(listenfd);


    return 0;
}
