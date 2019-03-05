#ifndef _ARCHIVES_H
#define _ARCHIVES_H

typedef struct ManageOpr {
	char name[10];
    char account[10];
    char password[10];
    char loged;
}T_ManageOpr, *PT_ManageOpr;

#define MEMLEND 1000

extern int Archives_Init(void);
extern T_ManageOpr *LogManger(char * account,char * password);
extern void unLogManger(char * account);

#endif

