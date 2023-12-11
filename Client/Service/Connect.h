/*************************************************************************
	>    File Name: Connect.h
	>       Author: fujie
	>         Mail: fujie.me@qq.com
	> Created Time: 2017年08月10日 星期四 17时32分47秒
 ************************************************************************/

#ifndef _CONNECT_H
#define _CONNECT_H
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include "../Common/Common.h"

/*
 * 创建并保持socket连接
 */
void Connect(const char *host, int port);

#endif
