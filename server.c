#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include "server.h"
#include "archives.h"

int iLogClientNum;
T_LogOpr LogTable[BACKLOG];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_send = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

const char logsuftitle[]="Log successfully!";
const char logerrtitle[]="Log error!";


void BroadMsg(char *str, int lend)      //���������ע����Ŀͻ��˷�������
{
    int i;

    for(i=0; i<BACKLOG; i++)
    {
        if(LogTable[i].used)
        {
            send(LogTable[i].socketid, str, strlen(str), 0);
        }
    }

}

T_LogOpr * RegiestLog(T_ManageOpr *pManage,int socketid)
{
    int i;
    char welcometitle[50];
    for(i=0; i<BACKLOG; i++)
    {
        if(!LogTable[i].used)
        {
            LogTable[i].socketid = socketid;
            memset(LogTable[i].name,0,sizeof(LogTable[i].name));
            memcpy(LogTable[i].name,pManage->name,strlen(pManage->name));
            memset(LogTable[i].account,0,sizeof(LogTable[i].account));
            memcpy(LogTable[i].account,pManage->account,strlen(pManage->account));
            LogTable[i].used = 1;
            break;
        }
    }
    iLogClientNum++;

    printf("welcome %s join our chat!\n",LogTable[i].name);
    sprintf(welcometitle,"welcome %s join our chat!",LogTable[i].name);
    pthread_mutex_lock(&mutex_send);
    BroadMsg(welcometitle,sizeof(welcometitle));
    pthread_mutex_unlock(&mutex_send);
    
    if(iLogClientNum>=BACKLOG)              //��ֹ�ͻ�����������������
    {
        printf("too much client!\n");
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condvar,&mutex);
    }
    return &LogTable[i];
}

void unRegiestLog(int socketid)
{
    int i;
    char Msg[20];

    for(i=0; i<BACKLOG; i++)
    {
        if(LogTable[i].socketid == socketid)
        {
            sprintf(Msg,"%s leave our chat!",LogTable[i].name);
            pthread_mutex_lock(&mutex_send);
            BroadMsg(Msg,sizeof(Msg));
            pthread_mutex_unlock(&mutex_send);
            printf("%s leave our chat!\n",LogTable[i].name);

            unLogManger(LogTable[i].account);
            
            LogTable[i].used=0;
            break;
        }
    }
    close(socketid);


    iLogClientNum --;
    pthread_cond_signal(&condvar);
    pthread_mutex_unlock(&mutex);
}

void *pthread_LogClientFun(void*arg)
{
    int iRecvLen,i;
    T_LogOpr *ClientOpr = (T_LogOpr *)arg;
    char ucRecvBuf[LEND];       //����ͻ��˷���������
    char title[20];

    memset(title,0,20);
    sprintf(title,"%s said:",ClientOpr->name);

    while(1)
    {
        iRecvLen = recv(ClientOpr->socketid, ucRecvBuf, 999, 0);      //�ȴ��ͻ��˷�������
        if (iRecvLen <= 0)      //�ͻ����Ѿ��Ͽ�
        {
            unRegiestLog(ClientOpr->socketid);        //�Ƴ����еĿͻ���
            break ;
        }
        else                    //�յ���������
        {
            ucRecvBuf[iRecvLen]=0;
            printf("%s %s\n",title,ucRecvBuf);
            pthread_mutex_lock(&mutex_send);
            BroadMsg(title,sizeof(title));      //�������ͻ��˹㲥����
            BroadMsg(ucRecvBuf,iRecvLen);
            pthread_mutex_unlock(&mutex_send);
        }
    }
}

void *pthread_log(void*arg)
{
    int iSocketClient = *(int *)arg;
    int iRecvLen;
    char ucBuffer[LEND];
    char account[10];
    char password[10];
    int ret;
    T_ManageOpr *pManage = NULL;

    iRecvLen = recv(iSocketClient, ucBuffer, 999, 0);
    if (iRecvLen <= 0)
    {
        close(iSocketClient);
        printf("connect error!\n");
        return NULL;
    }

    ucBuffer[iRecvLen] = 0;
    printf("%s\n",ucBuffer);

    memset(account,0,sizeof(account));
    memset(password,0,sizeof(password));
    ret = sscanf(ucBuffer,"begin account:%s password:%s end",account,password);
    if(ret !=2)
    {
        goto logerror;
    }

    pManage = LogManger(account,password);
    if(NULL == pManage)
    {
        goto logerror;
    }

    send(iSocketClient, logsuftitle, strlen(logsuftitle), 0);

    return RegiestLog(pManage,iSocketClient);      //������ע���¿ͻ���
logerror:
    send(iSocketClient, logerrtitle, strlen(logerrtitle), 0);
    return NULL;

}

int main(int argc, int **argv)
{
    int iSocketServer;      //�����socketID
    int iSocketclient;      //�ͻ���socketID
    int ret;
    int iSockAddrlen;       //socket�ṹ�峤��
    struct sockaddr_in SockAddrServer;
    struct sockaddr_in SockAddrClient;
    pthread_t tid;          //���߳�ID
    T_LogOpr *value_ptr = NULL;

    ret = Archives_Init();
    if(ret)
    {
        printf("Archives init successfully!\n");
        printf("%d massage have been reload!\n",ret);
    }
    else
    {
        printf("Archives init error!\n");
        return -1;
    }

    iLogClientNum = 0;

    iSockAddrlen = sizeof(struct sockaddr);

    iSocketServer = socket(AF_INET, SOCK_STREAM, 0);    //����һ��socket����

    SockAddrServer.sin_family = AF_INET;
    SockAddrServer.sin_port = htons(SERVERPORT);
    SockAddrServer.sin_addr.s_addr = INADDR_ANY;
    memset(SockAddrServer.sin_zero,0,8);
    ret = bind(iSocketServer,(const struct sockaddr *)&SockAddrServer, iSockAddrlen);        //�󶨶˿�
    if(-1 == ret)
    {
        printf("bind error!\n");
        return -1;
    }

    ret = listen(iSocketServer,BACKLOG);        //�����˿�
    if(-1 == ret)
    {
        printf("listen error!\n");
        return -1;
    }

    printf("server has been build successfully!\n");

    while (1)
    {
        iSocketclient = accept(iSocketServer, (struct sockaddr *)&SockAddrClient, &iSockAddrlen);       //�ȴ��ͻ�������

        if (-1 != iSocketclient)
        {
            pthread_create(&tid,NULL,pthread_log,(void*)&iSocketclient);
            pthread_join(tid, (void *)&value_ptr);

            if(NULL == value_ptr)
            {
                printf("account or password error!\n");
                continue;
            }

            pthread_create(&tid,NULL,pthread_LogClientFun,(void*)value_ptr);       //�½�һ�����߳����ڶԽ��¿ͻ���
            pthread_detach(tid);        //�������߳�

        }
    }
    return 0;
}




