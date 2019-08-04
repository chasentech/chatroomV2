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

#include "wrap.h"

using namespace std;

#define MAX_LEN 256
#define LISTEN_PORT 8888


bool is_running_main = true;
bool is_running_chld = true;

void signalstop(int sign_no)
{
    printf("catch INT signal!\n");
    if(sign_no == SIGINT)
    {
        is_running_main = false;
        is_running_chld = false;
    }
}
/*
void signalchld(int sign_no)
{
    printf("catch CHLD signal!\n");
    if(sign_no == SIGCHLD)
    {
        is_running_main = false;
    }
}
*/

int main()
{
    signal(SIGINT, signalstop);
    //signal(SIGCHLD, signalchld);

    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

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
        while (is_running_chld && fgets(buf, MAX_LEN, stdin) != NULL)
        {
            //if (!strncmp(buf, "quit", 4))
            //{
            //    break;
            //}

            //printf("input data: %s", buf);
            Write(sockfd, buf, strlen(buf));
            bzero(buf, strlen(buf));
        }

        printf("child exit!\n");
    }
    else //main process
    {
        while (is_running_main)
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

        //release source
        wait(NULL);
        Close(sockfd);
        cout << "main exit" << endl;
    }

    return 0;
}
