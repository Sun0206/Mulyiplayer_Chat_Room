#include "server.h"

int user_list[256] = {0}; 
int main(int argc,char *argv[])
{
    if(argc != 3)
    {
        printf("./main IP Port\n");
        exit(-1);
    }
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd == -1)//套接字创建失败
    {
        perror("creat socket failed");
        exit(-1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_aton(argv[1], &serv_addr.sin_addr);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int r = bind(sock_fd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
    if(r == -1)
    {
        perror("bind failed");
        close(sock_fd);
        exit(-1);
    }
    listen(sock_fd,1024);
    int i=0;
    while(1)
    {
        struct sockaddr_in client_addr;
        memset(&client_addr,0,sizeof(client_addr));
        socklen_t len = sizeof(client_addr);
        int *accept_res = (int *)malloc(sizeof(int));
        *accept_res = accept(sock_fd,(struct sockaddr *)&client_addr,&len);
        if(*accept_res > 0)//客户端连接成功
        {
            printf("clientIP:%s,clientPort:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            extern int user_list[256];
            user_list[i++] = *accept_res;
            pthread_t tid;
            pthread_create(&tid,NULL,my_routine,(void *)accept_res);
            //pthread_join(tid,NULL);
        }
    }
    close(sock_fd);
    return 0;
}

int Login_flag = -1;//登录成功/失败标识，-1：失败;0:成功
int Register_flag = -1;//注册成功/失败标识，-1：失败;0:成功
void database_conporation(int accept_res, char *username,char *password)
{
    sqlite3 *pd = NULL;
    int res = sqlite3_open("user_config.db",&pd);
    if(res != SQLITE_OK)
    {
        printf("open user_config failed!%d\n",res);
        return;
    }
    char sql[256] = {0};
    sprintf(sql,"SELECT * FROM user_config WHERE username='%s' and password='%s';",username,password);
    char *errmsg = NULL;
    int flag = 0;
    int r = sqlite3_exec(
            pd,                                  /* An open database */
            sql,                           /* SQL to be evaluated */
            callback,  /* Callback function */
            (void *)&flag,                                    /* 1st argument to callback */
            &errmsg                              /* Error msg written here */
        );
    if(r != SQLITE_OK)
    {
        printf("sqlite3_exec failed:%s\n",errmsg);
    }
    extern int Login_flag;
    char sent_message[64] = {0};
    if(Login_flag == 0)
    {
        sprintf(sent_message,"Login success!\n");
        //printf("Login success!\n");
    }
    else
    {
        sprintf(sent_message,"Login failed!check your username or password\n");
        //printf("Login failed!check your username or password\n");
    }
    write(accept_res,sent_message,strlen(sent_message));
    //3.关闭数据库
    sqlite3_close(pd);
}

int callback(void* arg,int argc,char* argv[],char* ssss[])
{
    *(int*)arg = 1;
    extern int Login_flag;
    Login_flag = 0;
    return 0;
}

void database_register(int accept_res, char *username,char *password)//用户注册函数
{
    sqlite3 *pd = NULL;
    int res = sqlite3_open("user_config.db",&pd);
    if(res != SQLITE_OK)
    {
        printf("open user_config failed!%d\n",res);
        return;
    }
    char sql[256] = {0};
    sprintf(sql,"INSERT INTO user_config(username,password) VALUES('%s','%s');",username,password);
    char *errmsg = NULL;
    int r = sqlite3_exec(
            pd,                                  /* An open database */
            sql,                           /* SQL to be evaluated */
            NULL,  /* Callback function */
            NULL,                                 /* 1st argument to callback */
            &errmsg                              /* Error msg written here */
        );
    if(r != SQLITE_OK)
    {
        printf("sqlite3_exec failed:%s\n",errmsg);
    }
    //3.关闭数据库
    sqlite3_close(pd);
}
int callback2(void* arg,int argc,char* argv[],char* ssss[])
{
    *(int*)arg = 1;
    extern int Register_flag;
    Register_flag = 0;
    return 1;
}
void *my_routine(void *arg)
{
    //线程分离
    pthread_detach(pthread_self());
    int accept_res = *((int *)arg);
    free(arg);
    char action[1] = {0};//存储客户端发送过来的操作代码(1:login,2:register,3:exit)
    char username[32] = {0};//存储客户端发送过来的用户名
    char password[32] = {0};//存储客户端发送过来的密码
    while(1)
    {
        char buf_recv[128] = {0};
        int ret;
        ret = read(accept_res,buf_recv,128);
        if(ret < 0)
        {
            perror("read message from client failed");
            pthread_exit(NULL);
        }
        //一个字节去读取接收到的内容，按照协议存储在对应的空间中
        char buf_byte;
        int i = 0;
        int j = 0;
        int k = 0;
        int flag = 0;
        buf_byte = buf_recv[i];
        action[0]=buf_byte;
        if(buf_byte == 0)//客户端为退出操作
        {
            break;
        }
        while(1)//客户端为 登录(1) 或者 注册功能(2)
        {
            i++;
            buf_byte = buf_recv[i];
            if(buf_byte == '/')
            {
                flag++;
            }
            if(flag == 1 && buf_recv[i+1] != '/')
            {
                username[j++] = buf_recv[i+1];
            }
            if(flag == 2 && buf_recv[i+1] != '/')
            {
                password[k++] = buf_recv[i+1];
            }
            if(flag == 3)
                break;
        }
        break;
    }
    if(action[0] == '1')
    {
        database_conporation(accept_res,username,password);
    }
    else if(action[0] == '2')
    {
        database_register(accept_res,username,password);
    }
    extern int Login_flag;
    if(Login_flag == 0)//用户登录成功，进入聊天室
    {
        printf("new IP join!\n");

        while(1)
        {
            char user_sent_message[256] = {0};//存储用户在聊天室发送的信息
            int res = read(accept_res,user_sent_message,256);
            if(res <= 0)
            {
                printf("read failed!\n");
                break;
            }
            char buf_sent[512] = {0};//存储用户名和他发送的信息，准备发送给其他客户端
            sprintf(buf_sent,"%s:%s",username,user_sent_message);
            extern int user_list[256];
            for(int i=0;i<256;i++)
            {
                if(user_list[i] == accept_res)
                {
                    continue;
                }
                else if(user_list[i]!=0 && user_list[i]!=1 && user_list[i]!=2)
                {
                    write(user_list[i],buf_sent,strlen(buf_sent));
                }
            }
        }
    }
}