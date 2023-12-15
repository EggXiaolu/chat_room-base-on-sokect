
#ifndef _GROUP_SRV_H
#define _GROUP_SRV_H
typedef struct group_t
{
    int gid;
    char name[30];
    int owner;     // 群主uid
    int num;       // 群成员数量
    int NewMsgNum; // 未读消息数量
    struct group_t *next;
} group_t;

int Group_Srv_Create(const char *gname);

int Group_Srv_AddMember(int gid, int uid);

// 获取群组列表
int Group_Srv_GetList();

/*
 * 在创建群聊或者被邀请加入群聊时，
 * 往本地群聊列表里添加服务器发来的群信息
 */
void Group_Srv_Join(const char *massage);

// 打印群成员
void Group_Srv_ShowMember(const char *massage);

// 获取群成员
void Group_Srv_GetMember(int gid);

// 退群、解散群
void Group_Srv_Quit(group_t *curGroup);

// 删除群成员
void Group_Srv_DelMembebr(group_t *curGroup);

void Group_Srv_Delete(const char *massage);
#endif
