#ifndef _EN_DE_CODE_H_
#define _EN_DE_CODE_H_

#include <iostream>
#include <stdio.h>

#define HEAD_OFFSET 0
#define HEAD_SIZE 1
#define LEN_OFFSET 1
#define LEN_SIZE 4
#define DATA_TYPE_OFFSET 5
#define DATA_TYPE_SIZE 4
#define CONTENT_OFFSET 9

typedef enum DataType
{
    DATA_COMMAND = 0, //command
    DATA_VERIFY,  //sign in or sign up
    DATA_CHAT,    //for chat
}DataType;

typedef enum VerifyType
{
    VERIFY_SIGN_IN = 0, //sign in
    VERIFY_SIGN_UP,  //sign up
}VerifyType;

typedef enum CommandType
{
    COMMAND_APPLY_CONNECT = 0,      //cli -->> ser,
    COMMAND_APPLY_CONNECT_SUCCESS,  //cli <<-- ser,
    COMMAND_APPLY_CONNECT_FAIL,     //cli <<-- ser,
    COMMAND_SIGN_IN_SUCCESS,        //cli <<-- ser,
    COMMAND_SIGN_IN_FAIL,           //cli <<-- ser,
    COMMAND_SIGN_UP_SUCCESS,        //cli <<-- ser,
    COMMAND_SIGN_UP_FAIL,           //cli <<-- ser,
    COMMAND_DIS_CONNECT,            //cli -->> ser,
    COMMANG_GET_FRIEND_LIST,        //cli -->> ser,
    COMMANG_SHOW_INFO,              //cli <<-- ser,
}CommandType;

typedef struct DataCommand
{
    CommandType command;
    char info[100];
}DataCommand;
typedef struct DataVerify
{
    char type;
    char name[20];
    char pswd[20];
}DataVerify;
typedef struct DataChat
{
    int chat_from;
    int chat_to;
    char data[190];
}DataChat;

typedef struct DataDesc
{
    DataType dataType;
    DataCommand dataCommand;
    DataVerify dataVerify;
    DataChat dataChat;
}DataDesc;

int encode_command(DataCommand dataCommand, char *buf);
int encode_verify(DataVerify dataVerify, char *buf);
int encode_chat(DataChat dataChat, char *buf);
void encode(DataDesc data, char *buf);
int decode_command(DataCommand *dataCommand, char *buf);
int decode_verify(DataVerify *dataVerify, char *buf);
int decode_chat(DataChat *dataChat, char *buf);
void decode(DataDesc *data, char *buf);
void printf_SendData(DataDesc *data);
void test_en_de_code();

#endif