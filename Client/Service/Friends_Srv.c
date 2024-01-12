
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./Friends_Srv.h"
#include <pthread.h>
#include "./Connect.h"
#include "../Common/Common.h"
#include "../Common/List.h"
#define MSG_LEN 1024
extern int gl_uid;
extern int sock_fd;
// extern pthread_mutex_t mutex;
extern int my_mutex;
// extern int your_mutex;
extern char msg[1024];
friends_t *FriendsList;

int Friends_Srv_GetList()
{
    int rtn;
    char snd_msg[1024];
    if (NULL != FriendsList)
    {
        List_Destroy(FriendsList, friends_t);
    }
    List_Init(FriendsList, friends_t);
    sprintf(snd_msg, "%c\t%d\t", 'G', gl_uid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    friends_t *newNode = NULL;
    while (1)
    {
        // pthread_mutex_lock(&mutex);
        My_Lock();
        newNode = (friends_t *)malloc(sizeof(friends_t));
        sscanf(msg + 2, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
               &newNode->uid, newNode->name, &newNode->sex,
               &newNode->is_vip, &newNode->is_online,
               &newNode->is_follow, &newNode->state);
        if (newNode->uid == 0)
        {
            My_Unlock();
            // pthread_mutex_unlock(&mutex);
            break;
        }
        newNode->NewMsgNum = 0;
        newNode->next = NULL;
        List_AddHead(FriendsList, newNode);
        My_Unlock();
        // pthread_mutex_unlock(&mutex);
    }
    // pthread_mutex_lock(&mutex);
    My_Lock();
    int res;
    sscanf(snd_msg + 2, "%d\t", &res);
    if (res == 1)
    {
        rtn = 1;
    }
    else
    {
        rtn = 0;
    }
    My_Unlock();
    // pthread_mutex_unlock(&mutex);
    return rtn;
}

int Friends_Srv_SendAdd(const char *fname)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%s", 'A', gl_uid, fname);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    My_Lock();
    int res, rtn;
    sscanf(msg + 2, "%d\t", &res);
    if (res)
    {
        printf("好友请求发送成功!");
        getchar();
        rtn = 1;
    }
    else
    {
        printf("请求失败: 用户不存在");
        getchar();
        rtn = 0;
    }
    My_Unlock();
    return rtn;
}

int Friends_Srv_SendDel(friends_t *f)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%s\t", 'D', gl_uid, f->name);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    My_Lock();
    int res, rtn;
    sscanf(msg + 2, "%d\t", &res);
    switch (res)
    {
    case 1:
        List_FreeNode(FriendsList, f, friends_t);
        printf("好友删除成功!");
        getchar();
        rtn = 1;
        break;
    case -1:
        printf("删除失败！");
        getchar();
        rtn = 0;
    }
    My_Unlock();
    return rtn;
}

int Friends_Srv_RecvAdd(const char *message)
{
    friends_t *newNode;
    newNode = (friends_t *)malloc(sizeof(friends_t));
    sscanf(message + 2, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
           &newNode->uid, newNode->name, &newNode->sex,
           &newNode->is_vip, &newNode->is_follow,
           &newNode->is_online, &newNode->state);
    newNode->NewMsgNum = 0;
    newNode->next = NULL;
    List_AddHead(FriendsList, newNode);
    if (newNode->state == 0)
        printf("\n%s 请求添加你为好友\n", newNode->name);
    else if (newNode->state == 1)
    {
        printf("\n%s 同意了你的好友请求\n", newNode->name);
    }
    return 1;
}

int Friends_Srv_RecvDel(const char *msg)
{
    int uid;
    sscanf(msg + 2, "%d", &uid);
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (f->uid == uid)
        {
            printf("\n%s 已将您删除!\n", f->name);
            List_FreeNode(FriendsList, f, friends_t);
            return 1;
        }
    }
    return 0;
}

int Friends_Srv_Apply(int uid, int fuid, int is_agree)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%d\t%d\t", 'a', uid, fuid, is_agree);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    return 1;
}

// 处理好友申请的反馈
int Friends_Srv_ApplyRes(const char *msg)
{
    if (msg[1] != '\n')
    {
        Friends_Srv_RecvAdd(msg);
        return 1;
    }
    printf("朋友拒绝了你的好友请求\n");
    return 1;
}
