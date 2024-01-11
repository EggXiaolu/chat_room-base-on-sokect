
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include "./Group_Srv.h"
#include "./Friends_Srv.h"
#include "../Common/cJSON.h"
#include "../Common/Common.h"
#include "../Common/List.h"
#define MSG_LEN 1024

extern int gl_uid;
extern int sock_fd;
extern int my_mutex;
extern char msg[1024];
extern friends_t *FriendsList;
extern group_t *curGroup;
group_t *GroupList;

int Group_Srv_Create(const char *gname)
{
    int rtn = 0;
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%s\t%d\t", 'c', gname, gl_uid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    My_Lock();
    int res;
    sscanf(msg + 2, "%d", &res);
    if (res == 1)
    {
        printf("群创建成功!");
        getchar();
        rtn = 1;
    }
    else
    {
        printf("创建失败 :群名已存在！");
        getchar();
        rtn = 0;
    }
    My_Unlock();
    return rtn;
}
int Group_Srv_GetList()
{
    int rtn;
    char snd_msg[1024];
    if (NULL != GroupList)
    {
        List_Destroy(GroupList, group_t);
    }
    List_Init(GroupList, group_t);
    sprintf(snd_msg, "%c\t%d\t", 'g', gl_uid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send: 请求服务器失败");
        return 0;
    }
    group_t *newNode = NULL;
    while (1)
    {
        My_Lock();
        newNode = (group_t *)malloc(sizeof(group_t));
        sscanf(msg + 2, "%d\t%s\t%d\t%d\t",
               &newNode->gid, newNode->name,
               &newNode->owner, &newNode->num);
        if (newNode->gid == 0)
        {
            My_Unlock();
            break;
        }
        newNode->next = NULL;
        List_AddHead(GroupList, newNode);
        My_Unlock();
    }
    My_Lock();
    int res;
    sscanf(msg + 2, "%d\t", &res);
    if (res == 1)
    {
        rtn = 1;
    }
    else
    {
        printf("请求失败");
        rtn = 0;
    }
    My_Unlock();
    // pthread_mutex_unlock(&mutex);
    return rtn;
}
int Group_Srv_AddMember(int gid, int uid)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%d\t", 'M', gid, uid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    My_Lock();
    int res;
    sscanf(msg + 2, "%d\t", &res);
    if (res == 1)
    {
        printf("邀请成功!");
        getchar();
    }
    else
    {
        printf("邀请失败,该成员已在当前群聊中!");
        getchar();
    }
    My_Unlock();
    return 1;
}
void Group_Srv_Join(const char *msg)
{
    group_t *newNode = (group_t *)malloc(sizeof(group_t));
    sscanf(msg + 2, "%d\t%s\t%d\t%d\t",
           &newNode->gid, newNode->name,
           &newNode->owner, &newNode->num);
    newNode->NewMsgNum = 0;
    newNode->next = NULL;
    List_AddHead(GroupList, newNode);
    if (newNode->owner == gl_uid)
        return;
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (newNode->owner == f->uid)
        {
            printf("\n%s 邀请你加入了群聊 %s\n", f->name, newNode->name);
            return;
        }
    }
}

void Group_Srv_ShowMember(const char *msg)
{
    friends_t GroupMember;
    sscanf(msg + 2, "%d\t%s\t%d\t%d\t%d\t%d\t",
           &GroupMember.uid, GroupMember.name,
           &GroupMember.sex, &GroupMember.is_vip,
           &GroupMember.is_online, &GroupMember.per);
    char *is_online[2] = {"●", "\e[32m●\e[0m"};
    char *is_vip[2] = {"", "\e[31m"};
    char *sex[2] = {"\e[35m♀\e[0m", "\e[36m♂\e[0m"};
    char *per[3] = {"", "[\e[32m管理员\e[0m]", "[\e[33m群主\e[0m]"};
    printf("   %s %s%s\e[0m %s %s\n",
           is_online[GroupMember.is_online],
           is_vip[GroupMember.is_vip],
           GroupMember.name, sex[GroupMember.sex],
           per[GroupMember.per]);
}

void Group_Srv_GetMember(int gid)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t", 'm', gid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
    }
}

void Group_Srv_Quit(group_t *curGroup)
{
    char choice[5];
    char snd_msg[1024];
    int doing;
    if (curGroup->owner == gl_uid)
    {
        // 解散群
        doing = 0;
        printf("您是群主,确认解散群聊 %s ?[yes/no]", curGroup->name);
    }
    else
    {
        doing = 1;
        printf("确认要退出群聊 %s ?[yes/no]", curGroup->name);
    }
    sprintf(snd_msg, "%c\t%d\t%d\t%d\t", 'Q', curGroup->gid, doing, gl_uid);
    sgets(choice, 5);
    if (strcmp(choice, "yes") != 0)
    {
        return;
    }
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
    }
    List_FreeNode(GroupList, curGroup, group_t);
    printf("操作成功！");
    getchar();
}

void Group_Srv_Delete(const char *msg)
{
    int gid, res;
    sscanf(msg + 2, "%d\t%d\t", &gid, &res);
    group_t *g;
    List_ForEach(GroupList, g)
    {
        if (g->gid == gid)
        {
            if (res == 0)
            {
                printf("\n群主已将群聊 %s 解散!\n", g->name);
            }
            else if (res == 1)
            {
                printf("\n群主已将您踢出 %s !\n", g->name);
            }
            List_FreeNode(GroupList, g, group_t);
            curGroup = NULL;
            return;
        }
    }
}

void Group_Srv_RemoveMember(group_t *curGroup, char *name)
{
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%s\t%d\t", 'd',
            curGroup->gid, name,
            curGroup->owner);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return;
    }
    My_Lock();
    int res;
    sscanf(msg + 2, "%d\t", &res);
    switch (res)
    {
    case 1:
        printf("成员删除成功\n");
        break;
    case -1:
        printf("无法删除群主\n");
        break;
    case -2:
        printf("你没有权限\n");
        break;
    case -3:
        printf("该用户不存在\n");
        break;
    }
    getchar();
    My_Unlock();
    return;
}
