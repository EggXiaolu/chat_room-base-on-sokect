
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include "./Connect.h"
#include "./Account_Srv.h"
#include "./Chat_Srv.h"
#include "./Friends_Srv.h"
#include "../Persistence/Account_Persist.h"
#include "../Persistence/Friends_Persist.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#define MSG_LEN 1024

extern online_t *OnlineList;
int Account_Srv_SendIsOnline(int uid, int is_online)
{
    int f_sock_fd;
    friends_t *FriendsList, *f;
    List_Init(FriendsList, friends_t);
    Friends_Perst_GetList(FriendsList, uid);
    List_ForEach(FriendsList, f)
    {
        if (f->is_online)
        {
            f_sock_fd = Chat_Srv_GetFriendSock(f->uid);
            if (f_sock_fd == -1)
                return 0;
            cJSON *root = cJSON_CreateObject();
            cJSON *item = cJSON_CreateString("I");
            cJSON_AddItemToObject(root, "type", item);
            item = cJSON_CreateNumber(uid);
            cJSON_AddItemToObject(root, "fuid", item);
            item = cJSON_CreateBool(is_online);
            cJSON_AddItemToObject(root, "is_online", item);
            char *out = cJSON_Print(root);
            cJSON_Delete(root);
            // printf("上线:%s\n",out);
            if (send(f_sock_fd, (void *)out, MSG_LEN, 0) <= 0)
            {
                perror("send 客户端响应失败");
                free(out);
                return 0;
            }
            free(out);
        }
    }
    List_Destroy(FriendsList, friends_t);
    return 1;
}

int Account_Srv_ChIsOnline(int uid, int is_online, int sock_fd)
{
    online_t *curPos;
    int rtn = 0;
    if (is_online)
    {
        List_ForEach(OnlineList, curPos)
        {
            if (curPos->uid == uid)
            {
                close(curPos->sock_fd);
                curPos->sock_fd = sock_fd;
                rtn = 1;
                goto per;
            }
        }
        curPos = (online_t *)malloc(sizeof(online_t));
        curPos->uid = uid;
        curPos->sock_fd = sock_fd;
        curPos->next = NULL;
        List_AddHead(OnlineList, curPos);
        rtn = 1;
    }
    else
    {
        List_ForEach(OnlineList, curPos)
        {
            if (curPos->sock_fd == sock_fd)
            {
                uid = rtn = curPos->uid;
                List_FreeNode(OnlineList, curPos, online_t);
                break;
            }
        }
    }
    if (uid == -1)
        return 0;
per:
    if (Account_Perst_ChIsOnline(uid, is_online) == 0)
        rtn = 0;
    return rtn;
}

int Account_Srv_Out(int sock_fd, char *JSON)
{
    int uid;
    int rtn;
    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "uid");
    uid = item->valueint;
    cJSON_Delete(root);
    rtn = Account_Srv_ChIsOnline(uid, 0, sock_fd);
    if (rtn != -1)
    {
        Account_Srv_SendIsOnline(uid, 0);
        // 向在线好友发送下线通知
    }
    root = cJSON_CreateObject();
    item = cJSON_CreateString("R");
    cJSON_AddItemToObject(root, "type", item);
    item = cJSON_CreateBool((rtn > 0));
    cJSON_AddItemToObject(root, "res", item);
    item = cJSON_CreateString("服务器异常 喵喵?");
    cJSON_AddItemToObject(root, "reason", item);
    char *out = cJSON_Print(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) <= 0)
    {
        // perror("send ")
        rtn = 0;
    }
    // cJSON_Delete(root);
    // free(out);
    return rtn;
}

int Account_Srv_SignIn(int sock_fd, char *msg)
{
    char name[64], password[64];
    char snd_msg[MSG_LEN];
    int sex;
    int res;
    sscanf(msg + 1, "\t%s\t%d\t%s\t", name, &sex, password);
    if (Account_Perst_IsUserName(name)) // 检查是否重复
    {
        res = -1;
    }
    else
    {
        if (Account_Perst_AddUser(name, sex, password))
        {
            res = 1;
        }
        else
        {
            res = -2;
        }
    }
    sprintf(snd_msg, "%c\t%d\t", 'R', res);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        // 出错 日志处理
    }
    return 0;
}

int Account_Srv_Login(int sock_fd, char *msg)
{
    char snd_msg[MSG_LEN];
    char name[64], password[64];
    int res;
    int uid;
    sscanf(msg + 1, "\t%s\t%s\t", name, password);
    if ((uid = Account_Perst_IsUserName(name)) == 0)
    {
        res = -1;
    }
    else
    {
        // 用户名存在
        if (Account_Perst_MatchUserAndPassword(uid, password))
        {
            // Account_Srv_ChIsOnline(uid, 1, sock_fd); // 推送上线
            res = 1;
        }
        // 密码错的
        else
        {
            res = -2;
        }
    }
    sprintf(snd_msg, "%c\t%d\t%d\t", 'R', res, uid);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) < 0)
    {
        // 出错 日志处理
    }
    return 0;
}
