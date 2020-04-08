#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "en_de_code.h"

int encode_command(DataCommand dataCommand, char *buf)
{
    int len = 4;
    memcpy(&buf[CONTENT_OFFSET], &dataCommand.command, 4);
    return len;
}
int encode_verify(DataVerify dataVerify, char *buf)
{
    int len = 0;
    buf[CONTENT_OFFSET] = dataVerify.type;
    strcpy((char *)&buf[1+CONTENT_OFFSET], (char *)&dataVerify.name);
    len = strlen((char *)dataVerify.name);
    buf[1+CONTENT_OFFSET+len] = '|';
    strcpy((char *)&buf[1+CONTENT_OFFSET+1+len], (char *)&dataVerify.pswd);
    len += strlen((char *)dataVerify.pswd);
    return len + 1 + 1;
}
int encode_chat(DataChat dataChat, char *buf)
{
    int len = 0;
    memcpy(&buf[CONTENT_OFFSET], &dataChat.chat_from, 4);
    memcpy(&buf[CONTENT_OFFSET+4], &dataChat.chat_to, 4);
    strcpy((char *)&buf[CONTENT_OFFSET+4+4], (char *)&dataChat.data);
    len = strlen((char *)dataChat.data);
    return len + 8;
}

void encode(DataDesc data, char *buf)
{
    buf[HEAD_OFFSET] = '#';
    int encode_len = 0;
    DataType dataType = DATA_COMMAND;

    if (data.dataType == DATA_COMMAND)
    {
        dataType = DATA_COMMAND;
        memcpy(&buf[DATA_TYPE_OFFSET], &dataType, DATA_TYPE_SIZE);
        encode_len = encode_command(data.dataCommand, buf);
    }
    if (data.dataType == DATA_VERIFY)
    {
        dataType = DATA_VERIFY;
        memcpy(&buf[DATA_TYPE_OFFSET], &dataType, DATA_TYPE_SIZE);
        encode_len = encode_verify(data.dataVerify, buf);
    }
    if (data.dataType == DATA_CHAT)
    {
        dataType = DATA_CHAT;
        memcpy(&buf[DATA_TYPE_OFFSET], &dataType, DATA_TYPE_SIZE);
        encode_len = encode_chat(data.dataChat, buf);
    }

    buf[CONTENT_OFFSET + encode_len] = '$';
    int iiiii = CONTENT_OFFSET + 1 + encode_len; //MAX 256
    memcpy(&buf[LEN_OFFSET], &iiiii, LEN_SIZE);
}

int decode_command(DataCommand *dataCommand, char *buf)
{
    memcpy(&dataCommand->command, &buf[CONTENT_OFFSET], 4);
    return 0;
}
int decode_verify(DataVerify *dataVerify, char *buf)
{
    int len = 0;
    memcpy(&len, &buf[LEN_OFFSET], LEN_SIZE);
    dataVerify->type = buf[CONTENT_OFFSET];

    int name_offset = CONTENT_OFFSET + 1;
    int name_len;
    int pswd_offset;
    int pswd_len;

    int i = 0;
    for (i = CONTENT_OFFSET; buf[i] != '$'; i++)
    {
        if (buf[i] == '|')
        {
            name_len = i - name_offset;
            pswd_offset = i + 1;
            break;
        }
    }

    pswd_len = len - pswd_offset - 1;

    strncpy((char *)&dataVerify->name, (char *)&buf[name_offset], name_len);
    strncpy((char *)&dataVerify->pswd, (char *)&buf[pswd_offset], pswd_len);

    return 0;
}
int decode_chat(DataChat *dataChat, char *buf)
{
    int len = 0;
    memcpy(&len, &buf[LEN_OFFSET], LEN_SIZE);

    memcpy(&dataChat->chat_from, &buf[CONTENT_OFFSET], 4);
    memcpy(&dataChat->chat_to, &buf[CONTENT_OFFSET+4], 4);

    strncpy((char *)&dataChat->data, (char *)&buf[CONTENT_OFFSET+4+4], len - CONTENT_OFFSET+4+4);
    return len + 8;
}

void decode(DataDesc *data, char *buf)
{
    if (buf[HEAD_OFFSET] == '#')
    {
        int len = 0;
        memcpy(&len, &buf[LEN_OFFSET], LEN_SIZE);
        if (buf[len - 1] != '$')
        {
            printf("check buf failed: len error!\n");
            return;
        }
    }
    else
    {
        printf("check buf failed: head error!\n");
        return;
    }

    int encode_type = -1;
    memcpy(&encode_type, &buf[DATA_TYPE_OFFSET], DATA_TYPE_SIZE);
    data->dataType = (DataType)encode_type;

    if (encode_type == DATA_COMMAND)
        decode_command(&data->dataCommand, buf);

    if (encode_type == DATA_VERIFY)
        decode_verify(&data->dataVerify, buf);

    if (encode_type == DATA_CHAT)
        decode_chat(&data->dataChat, buf);
}

void printf_SendData(DataDesc *data)
{
    printf(">>------------------------------------------<<\n");
    printf("dataType  = %d\n", data->dataType);
    printf("DataCommand:\n");
    printf("    command = %d\n", data->dataCommand.command);
    printf("dataVerify:\n");
    printf("    type    = %d\n", data->dataVerify.type);
    printf("    name    = %s\n", data->dataVerify.name);
    printf("    pswd    = %s\n", data->dataVerify.pswd);
    printf("dataChat:\n");
    printf("chat_from   = %d\n", data->dataChat.chat_from);
    printf("chat_to     = %d\n", data->dataChat.chat_to);
    printf("data        = %s\n", data->dataChat.data);
    printf(">>------------------------------------------<<\n");
}
