typedef struct friends
{
    int uid;
    char name[30];
    int sex;
    int is_vip;    // 是否是会员
    int is_follow; // 是否为特别关心
    int is_online; // 是否在线
    int NewMsgNum; // 未读消息数
    int state;
    struct friends *next;
} friends_t;