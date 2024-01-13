
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include "./Connect.h"
#include "./Chat_Srv.h"
#include "../Common/cJSON.h"
#include "../Common/List.h"
#include "../Service/Friends_Srv.h"
#include "../Service/Group_Srv.h"
#define MSG_LEN 1024
extern int gl_uid;
extern int sock_fd;
extern friends_t *FriendsList;
extern group_t *GroupList;
private_msg_t *PriMsgList;
group_msg_t *GroMsgList;

int length(int n)
{
    int sum = 0;
    while (n)
    {
        n /= 10;
        sum++;
    }
    return sum;
}

int Chat_Srv_RecvFile(const char *msg)
{
    int uid, fuid, size;
    char filename[64];
    char fp[100] = "RecvFile/";
    sscanf(msg + 2, "%d\t%d\t%s\t%d\t", &uid, &fuid, filename, &size);
    char *buf = msg + 2 + length(uid) + 1 + length(fuid) + 1 + strlen(filename) + 1 + length(size) + 1;
    strcat(fp, filename);
    int fd = open(fp, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        perror("open");
        return 0;
    }
    if (write(fd, buf, size) != size)
    {
        perror("write");
        if (size < 512)
        {
            friends_t *f;
            List_ForEach(FriendsList, f)
            {
                if (f->uid == uid)
                {
                    printf("\n%s 发来一个文件,已保存至./RecvFile/%s\n",
                           f->name, basename(filename));
                    break;
                }
            }
        }
    }
    close(fd);
    return 0;
}

int Chat_Srv_SendFile(const char *filename, int fuid)
{
    char buf[650];
    int fd, size;
    char fp[64];
    strcpy(fp, "./file/");
    strcat(fp, filename);
    if ((fd = open(fp, O_RDONLY | 0)) == -1)
    {
        printf("文件不存在或无读取权限");
        return 0;
    }
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        size = read(fd, buf, sizeof(buf) - 2);
        base64_encodestate state_in;
        base64_init_encodestate(&state_in);
        memset(code_out, 0, sizeof(code_out));
        base64_encode_block(buf, size, code_out, &state_in);
        if (state_in.step != step_A)
        {
            // 如果不是base64编码
            memset(code_end, 0, sizeof(code_end));
            base64_encode_blockend(code_end, &state_in);
            strcat(code_out, code_end);
        }
        char snd_msg[1024];
        snprintf(snd_msg, sizeof(snd_msg), "%c\t%d\t%d\t%s\t%d\t", 'F', gl_uid, fuid, filename, size);
        memcpy(snd_msg + 2 * sizeof(int) + strlen(filename) + 2 * sizeof(int), buf, size);
        int ret;
        if ((ret = send(sock_fd, snd_msg, 2 + length(gl_uid) + 1 + length(fuid) + 1 + strlen(filename) + 1 + length(size) + 1, 0)) <= 0)
        {
            perror("send");
            return 0;
        }
        if (size < (int)sizeof(buf) - 2)
            break;
    }
    close(fd);
    return 1;
}

void Chat_Srv_InitList()
{
    List_Init(PriMsgList, private_msg_t);
    List_Init(GroMsgList, group_msg_t);
}

void Chat_Srv_RecvPrivate(const char *msg)
{
    int uid;
    private_msg_t *NewMsg = (private_msg_t *)malloc(sizeof(private_msg_t));
    sscanf(msg + 2, "%d\t%d\t%s\t%s", &NewMsg->from_uid, &uid, NewMsg->msg, NewMsg->time);
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (f->uid == NewMsg->from_uid)
        {
            strcpy(NewMsg->name, f->name);
            (f->NewMsgNum)++;
        }
    }
    NewMsg->next = NULL;
    private_msg_t *curPos;
    List_AddTail(PriMsgList, curPos, NewMsg);
    printf("\n%s 发来一条消息\n", NewMsg->name);
}

void Chat_Srv_RecvGroup(const char *msg)
{
    group_msg_t *NewMsg = (group_msg_t *)malloc(sizeof(group_msg_t));
    sscanf(msg + 2, "%d\t%d\t%s\t%s\t%s", &NewMsg->from_uid, &NewMsg->gid, NewMsg->msg, NewMsg->time, NewMsg->uname);
    group_t *g;
    List_ForEach(GroupList, g)
    {
        if (g->gid == NewMsg->gid)
        {
            strcpy(NewMsg->gname, g->name);
            (g->NewMsgNum)++;
        }
    }
    NewMsg->next = NULL;
    group_msg_t *curPos;
    List_AddTail(GroMsgList, curPos, NewMsg);
    printf("\n群聊 %s 有一条新消息\n", NewMsg->gname);
}

int Chat_Srv_SendPrivate(int to_uid, const char *msg)
{
    int rtn = 1;
    private_msg_t *NewMsg =
        (private_msg_t *)malloc(sizeof(private_msg_t));
    user_date_t Srvdate = DateNow();
    user_time_t Srvtime = TimeNow();
    char Srvdatetime[25];
    sprintf(Srvdatetime, "%04d-%02d-%02d %02d:%02d:%02d",
            Srvdate.year, Srvdate.month, Srvdate.day,
            Srvtime.hour, Srvtime.minute, Srvtime.second);
    strcpy(NewMsg->time, Srvdatetime);
    NewMsg->from_uid = gl_uid;
    strcpy(NewMsg->msg, msg);
    NewMsg->next = NULL;
    private_msg_t *m;
    List_AddTail(PriMsgList, m, NewMsg);
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%d\t%s\t", 'P', gl_uid, to_uid, msg);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        printf("服务器失去响应\n");
        rtn = 0;
    }
    return rtn;
}

int Chat_Srv_SendGroup(int to_gid, const char *msg)
{
    int rtn = 1;
    group_msg_t *NewMsg =
        (group_msg_t *)malloc(sizeof(group_msg_t));
    NewMsg->gid = to_gid;
    user_date_t Srvdate = DateNow();
    user_time_t Srvtime = TimeNow();
    char Srvdatetime[25];
    sprintf(Srvdatetime, "%04d-%02d-%02d %02d:%02d:%02d",
            Srvdate.year, Srvdate.month, Srvdate.day,
            Srvtime.hour, Srvtime.minute, Srvtime.second);
    strcpy(NewMsg->time, Srvdatetime);
    NewMsg->from_uid = gl_uid;
    strcpy(NewMsg->msg, msg);
    NewMsg->next = NULL;
    group_msg_t *m;
    List_AddTail(GroMsgList, m, NewMsg);
    char snd_msg[1024];
    sprintf(snd_msg, "%c\t%d\t%d\t%s\t", 'p', gl_uid, to_gid, msg);
    if (send(sock_fd, snd_msg, MSG_LEN, 0) <= 0)
    {
        printf("服务器失去响应\n");
        rtn = 0;
    }
    return rtn;
}

void Chat_Srv_ShowPrivateRec(const char *JSON)
{
    private_msg_t *NewMsg = (private_msg_t *)malloc(sizeof(private_msg_t));

    cJSON *root = cJSON_Parse(JSON);
    cJSON *item = cJSON_GetObjectItem(root, "from_uid");
    NewMsg->from_uid = item->valueint;
    item = cJSON_GetObjectItem(root, "msg");
    strcpy(NewMsg->msg, item->valuestring);
    item = cJSON_GetObjectItem(root, "time");
    strcpy(NewMsg->time, item->valuestring);
    friends_t *f;
    List_ForEach(FriendsList, f)
    {
        if (f->uid == NewMsg->from_uid)
        {
            strcpy(NewMsg->name, f->name);
            (f->NewMsgNum)++;
        }
    }
    NewMsg->next = NULL;
    cJSON_Delete(root);
    if (NewMsg->from_uid == gl_uid)
    {
        printf("\t\e[32m%s\e[0m ", NewMsg->time);
        printf("我\n");
        printf("\t  \e[1m%s\e[0m\n", NewMsg->msg);
    }
    else
    {
        printf("\t\e[31m%s\e[0m ", NewMsg->time);
        printf("%s\n", NewMsg->name);
        printf("\t  \e[1m%s\e[0m\n", NewMsg->msg);
    }
}

void Chat_Srv_GetPrivateRec(int fuid)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "E");
    cJSON_AddNumberToObject(root, "uid", gl_uid);
    cJSON_AddNumberToObject(root, "fuid", fuid);
    char *out = cJSON_Print(root);
    cJSON_Delete(root);
    if (send(sock_fd, (void *)out, MSG_LEN, 0) <= 0)
    {
        printf("服务器失去响应\n");
    }
    free(out);
    system("clear");
    sleep(1);
}
