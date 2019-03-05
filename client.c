#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#define SERVERPORT 1234
pthread_rwlock_t rwlock;    //声明读写锁
unsigned char ucBuffer[1000];

char account[10];
char password[10];

const char logsuftitle[]="Log successfully!";
const char logerrtitle[]="Log error!";


void *pthread_send(void*arg)
{
    int iSocketClient = *(int *)arg;
    int iSendLen;
    while (1)
    {
        if (fgets(ucBuffer, 999, stdin))        //等待控制台输入数据
        {
            pthread_rwlock_wrlock(&rwlock);
            iSendLen = send(iSocketClient, ucBuffer, strlen(ucBuffer), 0);      //发送数据至服务器
            if (iSendLen <= 0)
            {
                close(iSocketClient);
                break;
            }
            pthread_rwlock_unlock(&rwlock);
        }
    }

}

void *pthread_log(void*arg)
{
    int iSocketClient = *(int *)arg;
    int iSendLen,iRecvLen;

    memset(ucBuffer,0,sizeof(ucBuffer));
    sprintf(ucBuffer,"begin account:%s password:%s end",account,password);

    iSendLen = send(iSocketClient, ucBuffer, strlen(ucBuffer), 0);      //发送数据至服务器
    if (iSendLen <= 0)
    {
        close(iSocketClient);
        printf("connect error!\n");
        return (void *)logerrtitle;
    }

    iRecvLen = recv(iSocketClient, ucBuffer, 999, 0);
    if (iRecvLen <= 0)
    {
        close(iSocketClient);
        printf("connect error!\n");
        return (void *)logerrtitle;
    }
    
    if(!strncmp(logsuftitle,ucBuffer,strlen(logsuftitle)))
    {
        return (void *)logsuftitle;
    }

}


int main(int argc, int **argv)
{
    int iSocketClient;
    int ret,iRecvLen;
    int iSockAddrlen;
    pthread_t tid;
    struct sockaddr_in SockAddrServer;
    char * value_ptr;

    if (argc != 3)
    {
        printf("Usage:\n");
        printf("%s <account> <password>\n", argv[0]);
        return -1;
    }

    memset(account,0,sizeof(account));
    memcpy(account,argv[1],strlen((const char *)argv[1]));

    memset(password,0,sizeof(password));
    memcpy(password,argv[2],strlen((const char *)argv[2]));

    iSocketClient = socket(AF_INET, SOCK_STREAM, 0);        //建立一个socket连接
    if(-1 == iSocketClient)
    {
        printf("creat socket error!\n");
        return -1;
    }

    SockAddrServer.sin_family = AF_INET;
    SockAddrServer.sin_port = htons(SERVERPORT);
    ret = inet_aton("192.168.9.207",&SockAddrServer.sin_addr);
    memset(SockAddrServer.sin_zero,0,8);
    ret = connect(iSocketClient, (struct sockaddr *)&SockAddrServer,sizeof(struct sockaddr));   //连接至服务器
    if(-1 == ret)
    {
        printf("connet error!\n");
        return -1;
    }

    pthread_create(&tid,NULL,pthread_log,(void*)&iSocketClient);       //创建一个子线程用于登录服务器
    pthread_join(tid, (void *)&value_ptr);

    if(strncmp(logsuftitle,value_ptr,strlen(logsuftitle)))
    {
        printf("account or password error!\n");
        return -1;
    }

    printf("join the chat successfully!\n");

    pthread_rwlock_init(&rwlock,NULL);      //初始化一个读写锁

    pthread_create(&tid,NULL,pthread_send,(void*)&iSocketClient);       //创建一个子线程用于处理发送的数据

    while (1)
    {
        iRecvLen = recv(iSocketClient, ucBuffer, 999, 0);
        pthread_rwlock_rdlock(&rwlock);
        if (iRecvLen <= 0)
        {
            close(iSocketClient);
            printf("server has been closed!\n", ucBuffer);
            return 0;
        }
        else
        {
            ucBuffer[iRecvLen] = '\0';
            printf("%s\n", ucBuffer);
        }
        pthread_rwlock_unlock(&rwlock);
    }

    return 0;

}


