
#ifndef _ACCOUNT_PERSIST_H
#define _ACCOUNT_PERSIST_H

/*
 * 修改用户的在线状态
 */
int Account_Perst_ChIsOnline(int uid, int is_online);

/*
 * 检测一个用户名是否存在
 * 参数传入一个用户名
 * 如果存在,返回uid,否则返回0
 */
int Account_Perst_IsUserName(const char *name);

/*
 * 将一条用户数据写入数据库
 * 参数传入一个用户名和密码
 * 写入成功返回1,否则返回0
 */
int Account_Perst_AddUser(const char *name, int sex, const char *password);

/*
 * 检测用户名(uid)密码是否匹配
 * 匹配返回1,不匹配返回0
 * uid通过Account_Perst_IsUserName()获得
 */
int Account_Perst_MatchUserAndPassword(int uid, const char *password);

// 根据uid查询用户名
char *Account_Perst_GetUserNameFromUid(int uid);

// 根据用户名查询uid
int Account_Perst_GetUserUidFromName(char *name);
#endif
