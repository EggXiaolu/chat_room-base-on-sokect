
#ifndef _MYSQL_H
#define _MYSQL_H

#include <mysql/mysql.h>

/*
 * 连接数据库
 */
int MySQL_Connect(const char *host, const char *user, const char *pass, const char *database, const int port);

/*
 * 清理资源,关闭连接
 */
void MySQL_Close();

#endif
