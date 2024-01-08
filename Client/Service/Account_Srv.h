#ifndef _ACCOUNT_SRV_H
#define _ACCOUNT_SRV_H
#include "../Common/cJSON.h"
#include "Connect.h"

// 用户基本信息
struct Account
{
    char type;
    char name[24];
    char password[24];
};

void Account_Srv_RecvIsOnline(char *JSON);

/*
 * 注销登录
 */
int Account_Srv_Out();

int Account_Srv_SignIn(const char *name, int sex, const char *password);

int Account_Srv_Login(const char *name, const char *password);

#endif
