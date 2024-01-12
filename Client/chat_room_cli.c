
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "Common/cJSON.h"
#include "./View/Main_UI.h"
#include "./Service/Connect.h"
extern int sock_fd;
int gl_uid; // 记录登录用户的uid
int main(int agc, char **argv)
{
    // 获取客户端配置
    int fd = open("config.json", O_RDONLY);
    if (fd == -1)
    {
        printf("配置文件打开失败!");
        getchar();
        exit(0);
    }
    if (agc <= 2)
    {
        printf("usage:\n"
               "    command <ip> <port>\n");
        exit(0);
    }
    char host[50];
    int port;
    strcpy(host, argv[1]);
    port = (unsigned short)atoi(argv[2]);
    close(fd);
    Connect(host, port);
    Main_UI_Hello(gl_uid);
    close(sock_fd);
}
