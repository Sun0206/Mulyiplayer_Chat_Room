#ifndef __CLIENT_H__
#define __CLIENT_H__

#include<stdio.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"

typedef struct message
{
    int operation;//操作类型：1登录，2注册，0退出
    char username[32];
    char password[32];
}Message;

int User_Login(Message *user,int sock_fd);//用户登录功能
void users_talkroom(int sock_fd);//进入服务器的聊天室
void *users_talkroom_recive(void *arg);
void User_Register(Message *user,int sock_fd);
#endif