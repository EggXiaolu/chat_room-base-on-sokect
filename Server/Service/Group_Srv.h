
#ifndef _GROUP_SRV_H
#define _GROUP_SRV_H
#include "./Friends_Srv.h"
typedef struct group_t
{
    int gid;
    char name[50];
    int owner; // 群主uid
    int num;   // 群成员数量
    struct group_t *next;
} group_t;

typedef struct group_member
{
    int gid;
    friends_t user_info;
    int permission;
    struct group_member *next;
} group_member_t;

// 邀请好友加群
int Group_Srv_AddMember(int client_fd, char *JSON);

// 创建群聊
int Group_Srv_Create(int client_fd, char *buf);

// 获取群列表
int Group_Srv_GetList(int client_fd, char *JSON);

// 将群信息发给被邀请者或创建者
void Group_Srv_SendInfo(int gid, int uid);

// 获取群成员
void Group_Srv_ShowMember(int client_fd, const char *JSON);

// 删除群成员
void Group_Srv_RemoveMember(int client_fd, const char *JSON);

// 退群
void Group_Srv_Quit(int client_fd, const char *JSON);

#endif
