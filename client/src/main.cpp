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



bool is_running = true;

void signalstop(int signum)
{
    is_running = false;
    printf("catch signal!\n");
}


int main()
{
    signal(SIGINT, signalstop);

    int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(LISTEN_PORT);

    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    char buf[MAX_LEN];
    while (is_running == true && fgets(buf, MAX_LEN, stdin) != NULL)
    {
        Write(sockfd, buf, strlen(buf));
        int n = Read(sockfd, buf, MAX_LEN);

        if (n == 0)
            printf("the other side has been closed.\n");
        else
            Write(STDOUT, buf, n);
    }

    //release source
    Close(sockfd);
    cout << "main exit" << endl;

    return 0;
}