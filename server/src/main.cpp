#include <iostream>
#include <vector>

#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>

#include "wrap.h"
#include "en_de_code.h"

using namespace std;

#define MAX_BUF_LEN 256
#define LISTEN_PORT 10500
#define MAX_CLI_NUM 100

typedef struct CliInfo
{
    char IP[INET_ADDRSTRLEN]; //INET_ADDRSTRLEN = 16
    int port;
    int sockfd;
}CliInfo;

vector<CliInfo> g_vec_cli_info;


void print_cli_info(vector<CliInfo> &vec)
{
    cout << "********<<cli_info<<********" << endl;
    cout << "client number: " << vec.size() << endl;
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        cout << "    cli[" << i << "]_IP:     " << vec[i].IP << endl
             << "    cli[" << i << "]_port:   " << vec[i].port << endl
             << "    cli[" << i << "]_sockfd: " << vec[i].sockfd << endl;
        cout << "----------------------------" << endl;
    }
}

void vec_rm_value(vector<CliInfo> &vec, int value)
{
    vector<CliInfo>::iterator it;
    for (it = vec.begin(); it != vec.end(); )
    {
        if (it->sockfd == value)
            it = vec.erase(it);
        else it++;
    }
}

void return_fd(vector<CliInfo> &vec, int fd)
{
    vector<CliInfo>::iterator it;
    for (it = vec.begin(); it != vec.end(); it++)
    {
        char buf[30] = {0};
        if (it->sockfd == fd)
            sprintf(buf, "********%d<--[you]\n", it->sockfd);
        else sprintf(buf, "********%d\n", it->sockfd);
        Write(fd, buf, strlen(buf));
    }
}

void printf_buf(char *buf)
{
    printf("[%c][%d], t1:[%d], t2:[%d]\n",
           buf[0], buf[1], buf[2], buf[3]);
    int to = -1;
    int n = sizeof(to);
    memcpy(&to, (const char *)(buf+4), n);
    printf("to: [%d]\n", to);
    printf("nmsg: [%d]\n", buf[4+n]);
    printf("msg: [%s]\n", buf+4+n+1);
}

void send_list(vector<CliInfo> &vec, int sockfd)
{
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        cout << "    cli[" << i << "]_IP:     " << vec[i].IP << endl
             << "    cli[" << i << "]_port:   " << vec[i].port << endl
             << "    cli[" << i << "]_sockfd: " << vec[i].sockfd << endl;
        cout << "----------------------------" << endl;
        char buf_tmp[100] = {0};
        sprintf(buf_tmp, "\nIP: [%s], Port: [%d]\n", vec[i].IP, vec[i].port);
        write(sockfd, buf_tmp, strlen(buf_tmp));
    }
}

int handle_command(DataCommand *data, int sockfd, char *buf)
{
    switch (data->command)
    {
        case COMMAND_APPLY_CONNECT:
            //...
            break;

        case COMMAND_APPLY_CONNECT_SUCCESS:
            //...
            break;

        case COMMAND_APPLY_CONNECT_FAIL:
            //...
            break;

        case COMMANG_GET_LIST_FRIEND:
            send_list(g_vec_cli_info, sockfd);
            break;

        default:
            break;
    }
    return 0;
}

int handle_verify(DataVerify *data, int sockfd, char *buf)
{
    DataDesc sendData;
    memset(&sendData, 0, sizeof(sendData));
    sendData.dataType = DATA_COMMAND;
    if (data->type == VERIFY_SIGN_IN)
    {
        //to de check
        sendData.dataCommand.command = COMMAND_SIGN_IN_SUCCESS;
    }
    if (data->type == VERIFY_SIGN_UP)
    {

    }

    encode(sendData, buf);
    int len = 0;
    memcpy(&len, &buf[LEN_OFFSET], LEN_SIZE);
    Write(sockfd, buf, len);
    memset(buf, 0, len);

    return 0;
}

int handle_chat(DataChat *data, int sockfd, char *buf)
{
    return 0;
}

int handle_deal(char *buf, int sockfd)
{
    char buf_send[MAX_BUF_LEN] = {0};
    DataDesc revData;
    memset(&revData, 0, sizeof(revData));
    decode(&revData, buf);
    printf_SendData(&revData);
    if (revData.dataType == DATA_COMMAND)
    {
        handle_command(&revData.dataCommand, sockfd, buf_send);
    }
    if (revData.dataType == DATA_VERIFY)
    {
        handle_verify(&revData.dataVerify, sockfd, buf_send);
    }
    if (revData.dataType == DATA_CHAT)
    {
        handle_chat(&revData.dataChat, sockfd, buf_send);
    }

    return 0;
}

static int listenfd = Socket(AF_INET, SOCK_STREAM, 0);

void signalstop(int signum)
{
    printf("catch signal!\n");
    // Close(listenfd);
    // g_vec_cli_info.clear();
    // exit(0);
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
    char buf_rev[MAX_BUF_LEN] = {0};
    printf("Accepting connections ...\n");

    while (1)
    {
        rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (nready <= 0)
        {
            perror("select error");
            break;
        }

        if (FD_ISSET(listenfd, &rset)) // new connect arived
        {
            CliInfo cli_info;
            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof(cliaddr);
            int connfd = Accept(listenfd,
                         (struct sockaddr *)&cliaddr, &cliaddr_len);

            inet_ntop(AF_INET, &cliaddr.sin_addr, str_IP, sizeof(str_IP));
            strcpy(cli_info.IP, str_IP);
            cli_info.port = ntohs(cliaddr.sin_port);
            cli_info.sockfd = connfd;

            printf("connect from %s at PORT %d\n",
                str_IP, ntohs(cliaddr.sin_port));


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

            g_vec_cli_info.push_back(cli_info);
            print_cli_info(g_vec_cli_info);
            //printf("max fd: %d\n", maxi + 1);

            if (--nready == 0) continue;
        }

        for (i = 0; i <= maxi; i++) //already connect sockfd
        {
            int sockfd = client_fd[i];
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset))
            {
                int n = Read(sockfd, buf_rev, MAX_BUF_LEN);
                if (n == 0)
                {
                    print_cli_info(g_vec_cli_info);
                    vec_rm_value(g_vec_cli_info, sockfd);
                    print_cli_info(g_vec_cli_info);

                    //cout << "client size: " << g_vec_cli_info.size() << endl;
                    printf("the other side has been closed.\n");
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client_fd[i] = -1;
                } else
                {
                    //printf("received buf: %s", buf);

                    handle_deal(buf_rev, sockfd);
                    memset(buf_rev, 0, n);
                }

                if (--nready == 0) break; //notice maxi and nready relationship, can early exit
            }
        }
    }

    printf("main exit!\n");
    g_vec_cli_info.clear();
    Close(listenfd);

    return 0;
}
