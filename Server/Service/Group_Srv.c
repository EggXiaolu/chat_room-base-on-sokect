
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "./Group_Srv.h"
#include "./Chat_Srv.h"
#include "../Common/List.h"
#include "../Common/cJSON.h"
#include "../Persistence/Group_Persist.h"
#include "../Persistence/Account_Persist.h"
#define MSG_LEN 1024
int Group_Srv_AddMember(int client_fd, char *msg)
{
    int gid, uid, res;
    sscanf(msg + 2, "%d\t%d\t", &gid, &uid);
    if (Group_Perst_AddMember(gid, uid))
    {
        res = 1;
        // 将群信息下发给被邀请的用户
        Group_Srv_SendInfo(gid, uid);
    }
    else
    {
        res = -1;
    }
    char snd_msg[1024];
    printf("res=%d\n", res);
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(client_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    return 1;
}
void Group_Srv_SendInfo(int gid, int uid)
{
    group_t *GroupInfo = Group_Perst_GetInfo(gid);
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t", 'J', GroupInfo->gid, GroupInfo->name, GroupInfo->owner, GroupInfo->num);
    int f_fd = Chat_Srv_GetFriendSock(uid);
    if (send(f_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return;
    }
}

int Group_Srv_Create(int client_fd, char *msg)
{
    char gname[30];
    int uid;
    int res = 1;
    sscanf(msg + 2, "%s\t%d\t", gname, &uid);
    if (Group_Perst_IsGroup(gname))
    {
        res = -1;
    }
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(client_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
    }
    if (res == 1)
    {
        Group_Srv_SendInfo(Group_Perst_Create(uid, gname), uid);
    }
    return 0;
}

int Group_Srv_GetList(int client_fd, char *msg)
{
    // char buf[MSG_LEN];
    int uid;
    char snd_msg[MSG_LEN];
    sscanf(msg + 2, "%d\t", &uid);
    group_t *GroupList = NULL;
    List_Init(GroupList, group_t);
    Group_Perst_GetMyGroup(GroupList, uid);
    group_t *curPos;
    List_ForEach(GroupList, curPos)
    {
        sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t",
                'l', curPos->gid, curPos->name,
                curPos->owner, curPos->num);
        if (send(client_fd, snd_msg, MSG_LEN, 0) < 0)
        {
            perror("send 客户端响应失败");
            return 0;
        }
    }
    // 发送一个gid为0的数据告诉客户端发送完成
    sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t", 'l', 0, "0", 0, 0);
    if (send(client_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send 客户端响应失败");
        return 0;
    }
    // 销毁链表
    List_Destroy(GroupList, group_t);
    sprintf(snd_msg, "%c\t%d\t", 'R', 1);
    if (send(client_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("recv: 客户端响应失败");
        return 0;
    }
    return 1;
}

void Group_Srv_ShowMember(int client_fd, const char *msg)
{
    int gid;
    sscanf(msg + 2, "%d\t", &gid);
    group_member_t *GroupMember;
    List_Init(GroupMember, group_member_t);
    Group_Perst_GetGroupMember(GroupMember, gid);
    group_member_t *m;
    List_ForEach(GroupMember, m)
    {
        char snd_msg[MSG_LEN];
        sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t%d\t%d\t", 'm',
                m->user_info.uid, m->user_info.name,
                m->user_info.sex, m->user_info.is_vip,
                m->user_info.is_online, m->permission);
        if (send(client_fd, snd_msg, MSG_LEN, 0) < 0)
        {
            perror("recv: 客户端响应失败");
            continue;
        }
    }
}

void Group_Srv_Quit(int client_fd, const char *msg)
{
    int gid, _do, uid;
    sscanf(msg + 2, "%d\t%d\t%d\t", &gid, &_do, &uid);
    int f_fd = 0;
    if (_do == 0)
    {
        group_member_t *GroupMember;
        List_Init(GroupMember, group_member_t);
        Group_Perst_GetGroupMember(GroupMember, gid);
        group_member_t *m;
        char snd_msg[1024];
        sprintf(snd_msg, "%c\n%d\t%d\t", 'D', gid, 0);
        List_ForEach(GroupMember, m)
        {
            if (m->permission == 2)
                // 略过群主
                continue;
            if ((f_fd = Chat_Srv_GetFriendSock(m->user_info.uid)) > 0)
            {
                if (send(f_fd, snd_msg, MSG_LEN, 0) <= 0)
                {
                    perror("send");
                }
            }
        }
        Group_Perst_Delete(gid);
    }
    else
    {
        Group_Perst_DeleteMember(gid, uid);
    }
}

void Group_Srv_RemoveMember(int client_fd, const char *msg)
{
    char name[30];
    int gid, owner;
    int res = 0;
    sscanf(msg + 2, "%d\t%s\t%d\t", &gid, name, &owner);
    int uid = Account_Perst_GetUserUidFromName(name);
    if (uid == owner)
    {
        res = -1;
        printf("无法删除群主\n");
    }
    else if (!Group_Perst_HavePermission(gid, owner))
    {
        res = -2;
        printf("你没有权限\n");
    }
    else if (!Group_Perst_FindGroupMember(gid, uid))
    {
        res = -3;
        printf("该用户不存在\n");
    }
    else
    {
        if (Group_Perst_DeleteMember(gid, uid))
        {
            res = 1;
            // 向该成员发送通知
            char snd_msg[1024];
            int f_fd;
            sprintf(snd_msg, "%c\n%d\t%d\t", 'D', gid, 1);
            if ((f_fd = Chat_Srv_GetFriendSock(uid)) > 0)
            {
                if (send(f_fd, snd_msg, MSG_LEN, 0) <= 0)
                {
                    perror("send");
                }
            }
            printf("成员删除成功\n");
        }
        else
        {
            res = -4;
            printf("未知错误\n");
        }
    }
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(client_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return;
    }
    return;
}