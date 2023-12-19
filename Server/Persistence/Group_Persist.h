
#ifndef _GROUP_PERSIST_H
#define _GROUP_PERSIST_H
#include "../Service/Group_Srv.h"
// 群名是否存在
int Group_Perst_IsGroup(const char *gname);

// 创建群
int Group_Perst_Create(int uid, const char *name);

// 添加成员
int Group_Perst_AddMember(int gid, int uid);

// 删除成员 (踢人或者退群)
int Group_Perst_DeleteMember(int gid, int uid);

// 删除群 同时删除群成员
int Group_Perst_Delete(int gid);

// 查找用户所在群
int Group_Perst_GetMyGroup(group_t *MyGroupList, int uid);

// 获取群成员列表
int Group_Perst_GetGroupMember(group_member_t *GroupMember, int gid);

// 查找成员
int Group_Perst_FindGroupMember(int gid, int uid);

// 查看成员是否拥有群权限，有权限返回1，否则返回0
int Group_Perst_HavePermission(int gid, int uid);
group_t *Group_Perst_GetInfo(int gid);

#endif
