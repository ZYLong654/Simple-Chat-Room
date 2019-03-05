#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "archives.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



T_ManageOpr ManageTable[10];
int ManageNum;

const char BeginTitle[]="begin";
const char EndTitle[]="end";

void RegistManger(char *pSrc)
{
    char *pFind = pSrc;
    int ret;
    while(1)
    {
        pFind = strstr(pFind,BeginTitle);
        if(NULL == pFind)
        {
            return;
        }

        ret = sscanf(pFind,"begin name:%s account:%s password:%s end",
                     &ManageTable[ManageNum].name,
                     &ManageTable[ManageNum].account,
                     &ManageTable[ManageNum].password);
        if(ret !=3)
        {
            return;
        }
        ManageNum ++;
        ManageTable[ManageNum].loged = 0;

        pFind = strstr(pFind,EndTitle);
        if(NULL == pFind)
        {
            return;
        }
    }
}
T_ManageOpr *LogManger(char * account,char * password)
{
    int i;
    for (i = 0; i < 10; ++i)
    {
        if(!strcmp(account,ManageTable[i].account))
        {
            if(strcmp(password,ManageTable[i].password))
            {
                return NULL;

            }
            else
            {
                if(ManageTable[i].loged)
                {
                    return NULL;
                }
                else
                {
                    ManageTable[i].loged = 1;
                    return &ManageTable[i];
                }
            }
        }
    }
    return NULL;
}
void unLogManger(char * account)
{
    int i;
    for (i = 0; i < 10; ++i)
    {
        if(!strcmp(account,ManageTable[i].account))
        {
            ManageTable[i].loged = 0;
        }
    }
}

int Archives_Init()
{
    int fd;
    char *membuf = NULL;
    int ifilelend;

    ManageNum = 0;

    fd = open("management.txt",O_RDONLY);
    if(-1 == fd)
    {
        printf("cann't open the management.txt!\n");
        return 0;
    }


    membuf = malloc(MEMLEND);

    ifilelend = read(fd,membuf,MEMLEND);

    RegistManger(membuf);

    free(membuf);
    membuf = NULL;

    return ManageNum;

}


