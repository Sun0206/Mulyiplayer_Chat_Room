#ifndef __SERVER_H__
#define __SERVER_H__

#include<stdio.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "sqlite3.h"
#include "pthread.h"
void database_conporation(int accept_res,char *username,char *password);
int callback(void* arg,int argc,char* argv[],char* ssss[]);
int callback2(void* arg,int argc,char* argv[],char* ssss[]);
void *my_routine(void *arg);
void database_register(int accept_res, char *username,char *password);//用户注册函数
#endif