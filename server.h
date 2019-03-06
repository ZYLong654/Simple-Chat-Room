#ifndef _SERVER_H
#define _SERVER_H

typedef struct LogOpr {
    int socketid;
    char name[10];
    char account[10];
    char used;
}T_LogOpr, *PT_LogOpr;


#define SERVERPORT 1234     //服务器端口

#define BACKLOG    10       //最大聊天人数

#define LEND 1000           //缓存数据长度



#endif

