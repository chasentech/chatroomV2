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

typedef enum MsgType
{
    READY_TYPE = 1,
    CHAT_TYPE,
}MsgType;

typedef enum ReadyMsgType
{
    READY_MSG_SIGN_IN = 1,
    READY_MSG_SIGN_ON = 2,
    READY_MSG_ADMIN = 3,
    READY_MSG_TEST = 4,
}ReadyMsgType;

typedef enum ChatMsgType
{
    CHAT_MSG_NORMAL = 1,
    CHAT_MSG_GET_ID = 2,
}ChatMsgType;

static void printf_menu()
{
    printf("Please select item:\n");
    printf("    1. sign in.\n");
    printf("    2. sign up.\n");
    printf("    3. administrator.\n");
    printf("    4. set.\n");
    printf("    5. exit.\n");
}

//是否存在'\n'，并将回车改为'\0'
int is_enter(char *p)
{
    while (*p != '\0')
    {
        if (*p == '\n')
        {
            *p = '\0';
            return 1;
        }

        p++;
    }
    return 0;
}

void input(char *buf, int len)
{
    int flag = 0;
    while (1)
    {
        char temp[20] = {0};
        if (flag == 0)
        {
            flag = 1;
            fgets(buf, len, stdin);
            if (is_enter(buf) == 1) return;
        }
        else
        {
            fgets(temp, sizeof(temp), stdin);
            if (is_enter(temp) == 1) return;
        }
    }
}

void encode(char *buf, MsgType type1, ChatMsgType type2,
            int to, char *msg)
{
    int len = 0;
    char *p_buf = buf;
    *p_buf = '#'; p_buf++; len++;  //head
    *p_buf = 'n'; p_buf++; len++;  //n,长度
    *p_buf = type1; p_buf++; len++; //type2
    *p_buf = type2; p_buf++; len++; //type2
    int n2 = sizeof(to);           //to
    memcpy(p_buf, &to, n2); p_buf += n2; len += n2;
    *p_buf = 1; p_buf++; len++;    //msg n
    strcpy(p_buf, msg); p_buf += strlen(msg); len += strlen(msg);
    *p_buf = '|'; p_buf++; len++;  //
    *p_buf = '$'; p_buf++; len++;

    buf[1] = len;
    if (len >= MAX_LEN)
    {
        printf("out of len!");
        memset(buf, 0, MAX_LEN);
        return;
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

void module_admin(int sockfd)
{
    char buf[MAX_LEN] = {0};
    printf("in admin module, input quit to exit\n");

    //get list of ID
    encode(buf, CHAT_TYPE, CHAT_MSG_GET_ID, 0, (char *)" ");
    printf_buf(buf);
    Write(sockfd, buf, buf[1]);
    printf("len=%d\n", buf[1]);
    printf("Please friend's ID:\n");

    char to_temp[10] = {0};
    input(to_temp, 10);
    if (atoi(to_temp) <= 0) //default is self
    {
        sprintf(to_temp, "%d", sockfd);
    }

    printf("your input is %s\n", to_temp);


    while(1)
    {
        printf("Input: ");
        char buf_msg[200] = {0};
        input(buf_msg, 200);
        if (!strncmp(buf_msg, "quit", 4)) return;


        encode(buf, CHAT_TYPE, CHAT_MSG_NORMAL,
                atoi(to_temp), buf_msg);

        printf_buf(buf);
        printf("len = %d\n", buf[1]);

        Write(sockfd, buf, buf[1]);
        bzero(buf, MAX_LEN);
    }
}

void signalstop(int sign_no)
{
    if(sign_no == SIGINT)
    {
        printf("Please exit in the right way!\n");
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

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed!");
        return -1;
    }
    if (pid == 0) //child process
    {
        printf("-----Welcome to chatroom-----\n");
        while (1)
        {
            printf_menu();
            printf("Input: ");

            //input select
            bool is_break = false;
            char buf_select[5] = {0};
            input(buf_select, 2);
            switch (buf_select[0])
            {
                case '1': printf("enter sign in\n"); break;
                case '2': printf("enter sign up\n"); break;
                case '3': printf("enter administrator\n"); module_admin(sockfd); break;
                case '4': printf("enter set\n"); break;
                case '5': printf("enter exit\n"); is_break = true; break;
                default: printf("invalid input\n"); break;
            }

            if (is_break) break;
        }
    }
    else //main process
    {
        char buf_rece[MAX_LEN] = {0};
        while (1)
        {
            int n = Read(sockfd, buf_rece, MAX_LEN);
            if (n == 0)
            {
                printf("the other side has been closed.\n");
                break;
            }
            else
            {
                Write(STDOUT_FILENO, buf_rece, n);
                bzero(buf_rece, n);
            }
        }

        Close(sockfd);
        wait(NULL);
        printf("main exit!\n");

    }

    return 0;
}
