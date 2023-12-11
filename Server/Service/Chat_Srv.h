
#ifndef _CHAT_SRV_H
#define _CHAT_SRV_H

void Chat_Srv_File(const char *JSON);

int Chat_Srv_GetFriendSock(int fuid);

int Chat_Srv_Private(int sock_fd ,const char *JSON);

int Chat_Srv_Group(int sock_fd ,const char *JSON);

void Chat_Srv_SendOfflienPrivateMsg(int uid);

void Chat_Srv_SendPrivateRes(int sock_fd ,const char *JSON);
#endif
