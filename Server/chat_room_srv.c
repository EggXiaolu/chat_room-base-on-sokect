
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "Common/cJSON.h"
#include "Service/Connect.h"
#include "Persistence/MySQL.h"
int main()
{
    
    char buf[1024];
    char host[50], user[30], pass[50], database[50];
    int db_port;
    int fd = open("config.json", O_RDONLY);
    // 获取数据库配置
    if (fd == -1)
    {
        printf("配置文件打开失败!");
        getchar();
        exit(0);
    }
    read(fd, buf, 1024);
    cJSON *root = cJSON_Parse(buf);
    cJSON *item;
    item = cJSON_GetObjectItem(root, "db_port"); // 获取数据库端口
    db_port = item->valueint;
    item = cJSON_GetObjectItem(root, "host"); // 获取数据库地址
    strcpy(host, item->valuestring);
    item = cJSON_GetObjectItem(root, "user"); // 获取数据库用户
    strcpy(user, item->valuestring);
    item = cJSON_GetObjectItem(root, "pass"); // 获取数据库密码
    strcpy(pass, item->valuestring);
    item = cJSON_GetObjectItem(root, "database"); // 获取数据库名称
    strcpy(database, item->valuestring);
    item = cJSON_GetObjectItem(root, "server_port"); // 获取服务器端口
    int server_port = item->valueint;
    
    close(fd);
    cJSON_Delete(root);
    // printf("%s %s %s %s %d\n", host, user, pass, database, port);
    if (MySQL_Connect(host, user, pass, database, db_port) == 0)
    {
        printf("数据库连接失败\n");
        exit(0);
    }
    Connect(server_port);
}
