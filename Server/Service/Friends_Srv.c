
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Account_Srv.h"
#include "Friends_Srv.h"
#include "Chat_Srv.h"
#include "../Persistence/Friends_Persist.h"
#include "../Persistence/Account_Persist.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#define MSG_LEN 1024

extern online_t *OnlineList;
int Friends_Srv_GetList(int sock_fd, const char *msg)
{
    char snd_msg[MSG_LEN];
    int uid;
    sscanf(msg + 2, "\t%d\t", &uid);
    friends_t *FriendsList = NULL;
    List_Init(FriendsList, friends_t);
    Friends_Perst_GetList(FriendsList, uid);
    friends_t *curPos;
    List_ForEach(FriendsList, curPos)
    {
        sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
                'L', curPos->uid, curPos->name, curPos->sex,
                curPos->is_vip, curPos->is_online,
                curPos->is_follow, curPos->state);

        if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
        {
            perror("send 客户端响应失败");
            return 0;
        }
    }
    // 发送一个uid为0的数据告诉客户端发送完成
    sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t",
            'L', 0, "0", 0, 0, 0, 0, 0, 0);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send 客户端响应失败");
        return 0;
    }
    // 销毁链表
    List_Destroy(FriendsList, friends_t);
    sprintf(snd_msg, "%c\t%d\t", 'R', 1);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("recv: 客户端响应失败");
        return 0;
    }
    Chat_Srv_SendOfflienPrivateMsg(uid); // 推送离线消息
    return 1;
}

int Friends_Srv_Add(int sock_fd, const char *msg)
{
    int uid, fuid, res = 1;
    char fname[64];
    sscanf(msg + 2, "%d\t%s", &uid, fname);
    fuid = Account_Perst_IsUserName(fname);
    if (fuid == 0)
    {
        res = -1;
    }
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }
    Friends_Perst_Add(uid, fuid);
    if (Chat_Srv_GetFriendSock(fuid) == 0)
        return 1;
    Friends_Srv_SendAdd(uid, fuid, 'A');
    return 1;
}

int Friends_Srv_SendAdd(int uid, int fuid, char type)
{
    int f_sock_fd = -1;
    friends_t *NewFriends = (friends_t *)malloc(sizeof(friends_t));
    NewFriends->uid = uid;
    Friends_Perst_GetFriendInfo(NewFriends);
    f_sock_fd = Chat_Srv_GetFriendSock(fuid);
    if (type != 'A')
    {
        NewFriends->state = 1;
    }
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t",
            type, NewFriends->uid, NewFriends->name,
            NewFriends->sex, NewFriends->is_vip,
            NewFriends->is_follow, NewFriends->is_online,
            NewFriends->state);
    if (send(f_sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        perror("send");
        return 0;
    }
    return 1;
}

int Friends_Srv_Apply(int sock_fd, const char *msg)
{
    int uid, fuid, is_agree;
    sscanf(msg + 2, "%d\t%d\t%d\t", &uid, &fuid, &is_agree);
    int f_sock_fd = Chat_Srv_GetFriendSock(uid);
    Friends_Perst_Apply(fuid, uid, is_agree);
    if (is_agree)
    {
        // 同意
        Friends_Srv_SendAdd(uid, fuid, 'a');
    }
    else
    {
        char snd_msg[MSG_LEN];
        sprintf(snd_msg, "%c\n", 'a');
        if (send(f_sock_fd, snd_msg, MSG_LEN, 0) <= 0)
        {
            perror("send");
            return 0;
        }
    }
    return 1;
}

int Friends_Srv_Del(int sock_fd, const char *msg)
{
    int uid;
    char fname[64];
    sscanf(msg + 2, "%d\t%s\t", &uid, fname);
    int fuid = Account_Perst_IsUserName(fname);
    int res = 1;
    if (fuid == 0)
    {
        res = -1;
    }
    char snd_msg[MSG_LEN];
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        perror("send");
        return 0;
    }

    Friends_Perst_Del(uid, fuid);
    return 1;
}
