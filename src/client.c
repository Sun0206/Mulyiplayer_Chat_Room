#include "client.h"

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        printf("./main IP Port\n");
        exit(-1);
    }
    int sock_fd = socket(AF_INET,SOCK_STREAM,0);//创建TCP套接字
    if(sock_fd == -1)//创建失败
    {
        perror("creat sockfd failed");
        exit(-1);
    }
    //下面是为连接服务器端作准备
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_aton(argv[1],&serv_addr.sin_addr);
    serv_addr.sin_port = htons(atoi(argv[2]));
    //end
    int connect_fd = connect(sock_fd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));//请求连接服务器
    if(connect_fd == -1)//连接失败
    {
        perror("connect server failed");
        close(sock_fd);
        exit(-1);
    }
    printf("connect server success!\n");
    while(1)
    {
        Message user;
        printf("Please puts operation username password\noperation '1' is login,'2' is register,'3' is exit\n");
        scanf("%d%s%s",&user.operation,user.username,user.password);
        getchar();
        int login_flag = 1;
        switch(user.operation)
        {
        case 1://登录操作
            login_flag = User_Login(&user,sock_fd);
            break;
        case 2://注册操作
            User_Register(&user,sock_fd);
            break;
        case 0://退出操作
            return 0;
            break;
        default:
            break;
        }
        if(login_flag == -1)//登录失败
        {
            break;
        }
        else if(login_flag == 0)//登录成功
        {
            users_talkroom(sock_fd);//进入服务器的聊天室
        }
        else
        {
            continue;
        }
    }
    
}

int User_Login(Message *user,int sock_fd)//用户登录功能:登录成功返回0，失败返回-1
{
    char buf[128] = {0};//存储用户的操作以及登录的账号、密码
    sprintf(buf,"%d/%s/%s/",user->operation,user->username,user->password);
    write(sock_fd,buf,strlen(buf));
    char buf_recv[64] = {0};
    int read_res;
    read_res = read(sock_fd,buf_recv,64);
    if(read_res < 0)
    {
        perror("read message from server failed");
    }
    printf("%s\n",buf_recv);
    if(strlen(buf_recv) > 30)
    {
        return -1;
    }
    return 0;
}
void users_talkroom(int sock_fd)//进入服务器的聊天室
{
    printf("you have joined the talking room\n");
    pthread_t pid;
    pthread_create(&pid,NULL,users_talkroom_recive,(void *)&sock_fd);
    while(1)
    {
        char user_sent_message[256] = {0};
        fgets(user_sent_message,256,stdin);
        write(sock_fd,user_sent_message,strlen(user_sent_message));
    }
}
void *users_talkroom_recive(void *arg)//聊天室的子线程，用于一直接收服务发送的信息并打印
{
    //线程分离
    pthread_detach(pthread_self());
    int sock_fd = *((int *)arg);
    while(1)
    {
        char user_recive_message[512] = {0};
        int res = read(sock_fd,user_recive_message,512);
        if(res <= 0)
        {
            break;
        }
        printf("%s\n",user_recive_message);
    }
}

void User_Register(Message *user,int sock_fd)//用户注册函数
{
    char buf[128] = {0};//存储用户的操作以及登录的账号、密码
    sprintf(buf,"%d/%s/%s/",user->operation,user->username,user->password);
    write(sock_fd,buf,strlen(buf));
    char buf_register_recive[64] = {0};
    int read_res = read(sock_fd,buf_register_recive,64);
    if(read_res < 0)
    {
        printf("read failed\n");
    }
    printf("%s\n",buf_register_recive);
}



