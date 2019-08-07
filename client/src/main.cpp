#include <iostream>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "wrap.h"

using namespace std;

#define MAX_LEN 256
#define LISTEN_PORT 8888


bool is_running = true;

void signalstop(int sign_no)
{
    if(sign_no == SIGINT)
    {
        printf("Please input quit to exit!\n");
    }
}


static int sockfd = Socket(AF_INET, SOCK_STREAM, 0);
void signalchld(int sign_no)
{
    if(sign_no == SIGCHLD)
    {
        //release source
        wait(NULL);
        Close(sockfd);

        printf("SIGCHLD to exit!\n");
        exit(0);
    }
}




int main()
{
    signal(SIGINT, signalstop);
    signal(SIGCHLD, signalchld);


    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(LISTEN_PORT);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char buf[MAX_LEN];
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed!");
        return -1;
    }
    if (pid == 0) //child process
    {
        while (fgets(buf, MAX_LEN, stdin) != NULL)
        {
            if (!strncmp(buf, "quit", 4))
            {
                break;
            }

            Write(sockfd, buf, strlen(buf));
            bzero(buf, strlen(buf));
        }
    }
    else //main process
    {
        while (1)
        {
            int n = Read(sockfd, buf, MAX_LEN);
            if (n == 0)
            {
                printf("the other side has been closed.\n");
                break;
            }
            else
            {
                Write(STDOUT_FILENO, buf, n);
                bzero(buf, n);
            }
        }

        Close(sockfd);
        printf("main exit!\n");

    }

    return 0;
}
