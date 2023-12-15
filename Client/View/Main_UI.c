
#include <stdio.h>
#include "./Main_UI.h"
#include "./Account_UI.h"
#include "./Friends_UI.h"
#include "./Chat_UI.h"
#include "./Group_UI.h"
#include "../Service/Account_Srv.h"
#include "../Service/Friends_Srv.h"
#include "../Service/Chat_Srv.h"
#include "../Service/Group_Srv.h"
extern int gl_uid;
void Main_UI_Hello()
{
    int choice;
    do
    {
        if (gl_uid > 0)
        {
            Main_UI_Menu();
        }
        system("clear");
        printf(
            "==============================\n"
            "*****欢迎使来到Eggy聊天室*****\n"
            "==============================\n");
        printf(
            "功能选项:\n"
            "\t1.登录\n"
            "\t2.注册\n"
            "\t3.退出\n"
            "请输入功能序号:");
        scanf("%d", &choice);
        ffflush();
        switch (choice)
        {
        case 1:
            gl_uid = Account_UI_Login();
            break;
        case 2:
            Account_UI_SignIn();
            break;
        case 3:
            Account_UI_Exit();
            return;
            break;
        default:
            printf("请输入正确的指令！\n");
            break;
        }
    } while (1);
}

void Main_UI_Menu()
{
    Chat_Srv_InitList();
    Friends_Srv_GetList();
    Group_Srv_GetList();
    char choice;
    do
    {
        system("clear");
        Friends_UI_ShowList();
        Group_UI_ShowList();
        Friends_UI_ShowApply();
        printf("--------------功能--------------\n");
        printf("1.选择好友|2.选择群聊|3.处理申请\n"
               "4.添加好友|5.删除好友|6.创建群聊\n"
               "7.注销登录\n");
        printf("--------------------------------\n"
               "请选择功能:");
        scanf("%c", &choice);
        if (choice == '\n')
            continue;
        ffflush();
        switch (choice)
        {
        case '1':
            Chat_UI_Private();
            break;
        case '2':
            Chat_UI_Group();
            break;
        case '3':
            Friends_UI_Apply();
            break;
        case '4':
            Friends_UI_Add();
            break;
        case '5':
            Friends_UI_Del();
            break;
        case '6':
            Group_UI_Create();
            break;
        case '7':
            Account_Srv_Out();
            break;
        }
    } while (choice != '7');
}
