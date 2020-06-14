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
    char name[20];
}CliInfo;

vector<CliInfo> g_vec_cli_info;

void print_cli_info(vector<CliInfo> &vec)
{
    cout << "********<<cli_info<<********" << endl;
    cout << "client number: " << vec.size() << endl;
    for (unsigned int i = 0; i < vec.size(); i++)
    {
        cout << "    cli[" << i << "]_name:   " << vec[i].IP << endl
             << "    cli[" << i << "]_port:   " << vec[i].port << endl
             << "    cli[" << i << "]_port:   " << vec[i].port << endl
             << "    cli[" << i << "]_sockfd: " << vec[i].sockfd << endl;
        cout << "----------------------------" << endl;
    }
    cout << endl;
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

int get_fd_by_name(vector<CliInfo> &vec, char *name)
{
    vector<CliInfo>::iterator it;
    for (it = vec.begin(); it != vec.end(); it++)
    {
        if (strcpy(it->name, name) == 0)
        {
            return it->sockfd;
        }
    }
    return -1;
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

// void send_list(vector<CliInfo> &vec, int sockfd)
// {
//     for (unsigned int i = 0; i < vec.size(); i++)
//     {
//         cout << "    cli[" << i << "]_IP:     " << vec[i].IP << endl
//              << "    cli[" << i << "]_port:   " << vec[i].port << endl
//              << "    cli[" << i << "]_sockfd: " << vec[i].sockfd << endl;
//         cout << "----------------------------" << endl;
//         char buf_tmp[100] = {0};
//         sprintf(buf_tmp, "\nIP: [%s], Port: [%d]\n", vec[i].IP, vec[i].port);
//         write(sockfd, buf_tmp, strlen(buf_tmp));
//     }
// }

int handle_command(DataCommand *dataCommand, int sockfd)
{
    bool is_send_list = false;
    switch (dataCommand->command)
    {
        // case COMMAND_APPLY_CONNECT:
        //     //check out of client num
        //     //
        //     sendData->dataType = DATA_COMMAND;
        //     sendData->dataCommand.command = COMMAND_APPLY_CONNECT_SUCCESS;
        //     break;

        case COMMANG_GET_FRIEND_LIST: 
            is_send_list = true;
            // sendData.dataType = DATA_COMMAND;
            // sendData.dataCommand.command = COMMANG_SHOW_INFO;
            // //send_list(g_vec_cli_info, sockfd);
            // CliInfo vec = g_vec_cli_info[0];
            // sprintf(sendData.dataCommand.info, "IP:%s, port:%d", vec.IP, vec.port);
            break;

        default:
            break;
    }

    char buf_send[MAX_BUF_LEN] = {0};
    DataDesc sendData;
    memset(&sendData, 0, sizeof(sendData));

    if (is_send_list == true)
    {
        sendData.dataType = DATA_COMMAND;
        sendData.dataCommand.command = COMMANG_SHOW_INFO;
        sprintf(sendData.dataCommand.info, "fd   name       IP               port");
        encode(sendData, buf_send);
        int len = 0;
        memcpy(&len, &buf_send[LEN_OFFSET], LEN_SIZE);
        Write(sockfd, buf_send, len);
        memset(&sendData, 0, sizeof(sendData));
        memset(buf_send, 0, len);
        usleep(10000);

        for (unsigned int i = 0; i < g_vec_cli_info.size(); i++)
        {
            CliInfo &vec = g_vec_cli_info[i];
            sendData.dataType = DATA_COMMAND;
            sendData.dataCommand.command = COMMANG_SHOW_INFO;
            sprintf(sendData.dataCommand.info, "%-4d %-10s %-16s %-6d", vec.sockfd, vec.name, vec.IP, vec.port);
            encode(sendData, buf_send);
            int len = 0;
            memcpy(&len, &buf_send[LEN_OFFSET], LEN_SIZE);
            Write(sockfd, buf_send, len);

            memset(&sendData, 0, sizeof(sendData));
            memset(buf_send, 0, len);
            usleep(10000);
        }
    }

    return 0;
}

int handle_verify(DataVerify *data)
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

    return 0;
}

int handle_chat(DataChat *dataChat, int sockfd)
{
    char buf_send[MAX_BUF_LEN] = {0};
    DataDesc sendData;
    memset(&sendData, 0, sizeof(sendData));

    sendData.dataType = DATA_COMMAND;
    sendData.dataCommand.command = COMMANG_SHOW_INFO;
    strcpy(sendData.dataCommand.info, dataChat->data);
    encode(sendData, buf_send);
    int len = 0;
    memcpy(&len, &buf_send[LEN_OFFSET], LEN_SIZE);
    printf("fd[%d]-->fd[%d]: %s\n", sockfd, dataChat->chat_to, dataChat->data);
    Write(dataChat->chat_to, buf_send, len);
    memset(&sendData, 0, sizeof(sendData));
    memset(buf_send, 0, len);

    return 0;
}

int handle_msg(char *buf_recv, int sockfd)
{
    printf("into handle_msg\n");
    DataDesc recvData;
    memset(&recvData, 0, sizeof(recvData));
    if (decode(&recvData, buf_recv) < 0)
    {
        printf("[ser] decode failed\n");
        return -1;
    }
    printf_SendData(&recvData);

    if (recvData.dataType == DATA_COMMAND)
    {
        // sendData.dataType = DATA_COMMAND;
        handle_command(&recvData.dataCommand, sockfd);
    }
    else if (recvData.dataType == DATA_VERIFY)
    {
        //handle_verify(&recvData.dataVerify);
    }
    else if (recvData.dataType == DATA_CHAT)
    {
        handle_chat(&recvData.dataChat, sockfd);
    }
    else
    {
        printf("invalid data\n");
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
    // test_en_de_code();
    // return 0;

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
    char buf_recv[MAX_BUF_LEN] = {0};
    // char buf_send[MAX_BUF_LEN] = {0};
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
        printf("[debug] nready = %d\n", nready);
        if (FD_ISSET(listenfd, &rset)) // new connect arived
        {
            printf("[debug] arived\n");
            CliInfo cli_info;
            struct sockaddr_in cliaddr;
            socklen_t cliaddr_len = sizeof(cliaddr);
            int connfd = Accept(listenfd,
                         (struct sockaddr *)&cliaddr, &cliaddr_len);

            inet_ntop(AF_INET, &cliaddr.sin_addr, str_IP, sizeof(str_IP));
            strcpy(cli_info.IP, str_IP);
            cli_info.port = ntohs(cliaddr.sin_port);
            cli_info.sockfd = connfd;
            sprintf(cli_info.name, "n%d", connfd);

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
            printf("[debug] already\n");
            int sockfd = client_fd[i];
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset))
            {
                int n = Read(sockfd, buf_recv, MAX_BUF_LEN);
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
                    handle_msg(buf_recv, sockfd);

                    //clear buff
                    memset(buf_recv, 0, n);
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
