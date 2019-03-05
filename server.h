#ifndef _SERVER_H
#define _SERVER_H

typedef struct LogOpr {
    int socketid;
    char name[10];
    char account[10];
    char used;
}T_LogOpr, *PT_LogOpr;


#define SERVERPORT 1234

#define BACKLOG    10

#define LEND 1000




#endif

