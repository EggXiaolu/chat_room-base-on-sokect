
#ifndef _FRIENDS_PERSIST_H
#define _FRIENDS_PERSIST_H
#include "../Service/Friends_Srv.h"
#include <mysql/mysql.h>
// 数据库中添加加友信息
int Friends_Perst_Add(int uid, int fuid);
// 删除好友
int Friends_Perst_Del(int uid, int fuid);
// 处理添加好友请求
int Friends_Perst_Apply(int uid, int fuid, int is_argee);
// 获取好友信息
int Friends_Perst_GetList(friends_t *FriendsList, int uid);
// 填充好友结构体中的信息
int Friends_Perst_GetFriendInfo(friends_t *Node);

#endif
