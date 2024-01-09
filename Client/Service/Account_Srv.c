
#include <stdio.h>
#include "Account_Srv.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "./Connect.h"
#include "./Friends_Srv.h"
#include "../Common/List.h"

extern int sock_fd;
// extern pthread_mutex_t mutex;
// extern pthread_cond_t cond;
extern int my_mutex;
extern char msg[1024];
extern friends_t *FriendsList;
extern int gl_uid;

void Account_Srv_RecvIsOnline(char *JSON)
{
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "is_online");
    int is_online = item->valueint;
    item = cJSON_GetObjectItem(root, "fuid");
    int fuid = item->valueint;
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (f->uid == fuid)
        {
            f->is_online = is_online;
            if (is_online)
                printf("\n%s 上线啦!\n", f->name);
            else
                printf("\n%s 已下线.\n", f->name);
        }
    }
}
/*
 * 注销登录
 */
int Account_Srv_Out()
{
    int rtn;
    cJSON *root = cJSON_CreateObject();
    cJSON *item = cJSON_CreateString("O");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateNumber(gl_uid);
    cJSON_AddItemToObject(root, "uid", item);
    char *out = cJSON_Print(root);
    if (send(sock_fd, (void *)out, 1024, 0) <= 0)
    {
        perror("send 请求服务器失败");
        rtn = 0;
    }
    gl_uid = 0;
    cJSON_Delete(root);
    free(out);
    // pthread_mutex_lock(&mutex);
    // pthread_cond_wait(&cond ,&mutex);
    My_Lock();
    root = cJSON_Parse(msg);
    item = cJSON_GetObjectItem(root, "res");
    if (item->valueint == 0)
    {
        item = cJSON_GetObjectItem(root, "reason");
        printf("注销失败: %s", item->valuestring);
        rtn = 0;
    }
    else
    {
        printf("注销成功,按任意键继续..");
        rtn = 1;
    }
    cJSON_Delete(root);
    My_Unlock();
    getchar();
    // pthread_mutex_unlock(&mutex);
    return rtn;
}

int Account_Srv_SignIn(const char *name, int sex, const char *password)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%s\t%d\t%s\t", 'S', name, sex, password);
    if (send(sock_fd, snd_msg, 1024, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    My_Lock();
    int res, rtn;
    sscanf(msg + 1, "\t%d\t", &res);
    switch (res)
    {
    case 1:
        printf("注册成功!按任意键继续...");
        getchar();
        rtn = 1;
        break;
    case -1:
        printf("注册失败: 用户名已存在");
        getchar();
        rtn = 0;
        break;
    case -2:
        printf("注册失败: 数据库错误");
        getchar();
        rtn = 0;
    }
    My_Unlock();
    return rtn;
}
int Account_Srv_Login(const char *name, const char *password)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%s\t%s\t", 'L', name, password);
    if (send(sock_fd, snd_msg, 1024, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    // 进程锁，等待主线程响应
    My_Lock();
    int res, uid;
    int rtn;
    sscanf(msg + 1, "\t%d\t%d\t", &res, &uid);
    switch (res)
    {
    case 1:
        printf("登录成功!请稍候..");
        fflush(stdout);
        rtn = 1;
        break;
    case -1:
        printf("登录失败: 用户名不存在");
        getchar();
        rtn = 0;
        break;
    case -2:
        printf("登录失败: 用户名或密码不正确");
        getchar();
        rtn = 0;
    }

    // 解锁
    My_Unlock();
    return rtn;
}
