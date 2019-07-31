#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header.h"
#include "client_header.h"

/*建群 */
int new_group(int fd, char *username, char *account)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    printf("input group name:\n");
    scanf("%s", body->msg_a);
    strcpy(body->msg_b, username);
    strcpy(body->msg_d, account);
    body->action = NEWGROUP;

    send_fd(body, fd);

    bzero(body, sizeof(struct message));
    recv_fd(body, fd);

    if (strcmp(body->msg_b, "success") == 0)
    {
        printf("group %s create success\n", body->msg_a);
        sleep(1);

        free(body);
        return SUCCESS;
    }
    else if (strcmp(body->msg_b, "fail") == 0)
    {
        printf("group %s create fail\n", body->msg_a);
        sleep(1);

        free(body);
        return FALSE;
    }
}

/*显示我的群 */
int show_groups(int fd, char *account)
{
    int i = 0;

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = SHOWGROUPS;
    strcpy(body->msg_a, account);

    send_fd(body, fd);

    while (1) //循环接收信息
    {
        i++;

        bzero(body, sizeof(struct message));

        recv_fd(body, fd);

        if (strcmp(body->msg_a, "end") == 0) //终止接收
            break;

        printf("\tgroupname:%s\tgroupaccount:%s\n", body->msg_a, body->msg_c);
    }

    if (i == 1)
        printf("no groups\n");

    sleep(4);

    free(body);

    return SUCCESS;
}

/*显示我管理的群 */
int show_mananger_group(int fd, char *account)
{
    int i = 0;

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = SHOWMANAGERGROUP;
    strcpy(body->msg_a, account);

    send_fd(body, fd);

    while (1) //循环接收信息
    {
        i++;

        bzero(body, sizeof(struct message));

        recv_fd(body, fd);

        if (strcmp(body->msg_a, "end") == 0) //终止接收
            break;

        printf("\tgroupname:%s\tgroupaccount:%s\n", body->msg_a, body->msg_c);
    }

    if (i == 1)
        printf("no manager groups\n");

    sleep(4);

    free(body);

    return SUCCESS;
}

/*加群 */
int insert_group(int fd, char *name, char *account)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = JOINGROUP;

    printf("input group account:\n");
    scanf("%s", body->msg_b);

    strcpy(body->msg_c, name);
    strcpy(body->msg_d, account);

    send_fd(body, fd);

    bzero(body, sizeof(struct message));
    recv_fd(body, fd);

    if (strlen(body->msg_a) != 0)
    {
        printf("join group success\n");
        sleep(1);
        free(body);
        return SUCCESS;
    }
    else
    {
        printf("join group fail\n");
        sleep(1);
        free(body);
        return FALSE;
    }
}

/*退群 */
int exit_group(int fd, char *name)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = EXITGROUP;

    printf("input group account:\n");
    scanf("%s", body->msg_b);
    strcpy(body->msg_c, name);

    send_fd(body, fd);

    free(body);

    return SUCCESS;
}

/*显示群成员 */
int show_group_mem(int fd, char *useraccount)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = SHOWGROUPMEM;
    strcpy(body->msg_a, useraccount);
    send_fd(body, fd);

    while (1) //循环接收信息
    {
        bzero(body, sizeof(struct message));

        recv_fd(body, fd);

        if (strcmp(body->msg_a, "end") == 0) //终止接收
            break;

        if (strcmp(body->msg_a, "fail") == 0) //没有群
        {
            printf("no group\n");
            sleep(1);
            break;
        }

        printf("\tname:%s\taccount:%s\n", body->msg_a, body->msg_b);
    }

    sleep(4);

    free(body);
    return SUCCESS;
}

/*显示我管理的群成员 */
int show_mangroup_mem(int fd, char *useraccount)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = SHOWMANGROUPMEM;
    strcpy(body->msg_a, useraccount);
    send_fd(body, fd);

    while (1) //循环接收信息
    {
        bzero(body, sizeof(struct message));

        recv_fd(body, fd);

        if (strcmp(body->msg_a, "end") == 0) //终止接收
            break;

        if (strcmp(body->msg_a, "fail") == 0) //没有群
        {
            printf("no group\n");
            sleep(1);
            break;
        }

        printf("\tname:%s\taccount:%s\n", body->msg_a, body->msg_b);
    }

    sleep(4);

    free(body);
    return SUCCESS;
}

/*发送群消息 */
int send_groupmsg(int fd, char *username)
{
    char now[SIZE];
    char temp[SIZE];

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = SENDGROUPMSG;

    printf("input group name:\n");
    scanf("%s", body->msg_a);
    strcpy(body->msg_c, body->msg_a);
    strcat(body->msg_c, ":\t");
    strcat(body->msg_c, username); //c存群以及发送人
    strcpy(body->msg_d, username);

    while (1)
    {
        bzero(temp, sizeof(temp));
        printf("input send message:\n");

        bzero(now, sizeof(now));
        getTime(now); //获得并显示时间
        strcat(now, "\n");

        scanf("%s", temp); //写入消息

        if (strcmp(temp, "exit") == 0) //输入exit退出聊天
            break;

        strcat(now, temp);
        strcpy(body->msg_b, now); //b存时间

        send_fd(body, fd);
    }

    free(body);
    return SUCCESS;
}

/*踢人 */
int delete_groupmem(int fd, char *account)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = DELETEGROUPMEM;

    printf("input group name:\n");
    scanf("%s", body->msg_a);
    printf("input member name you want delete:\n");
    scanf("%s", body->msg_b);
    strcpy(body->msg_c, account);

    send_fd(body, fd);

    free(body);
    return SUCCESS;
}

/*禁言 */
int ban_groupuser(int fd, char *account)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = BANGROUPUSER;

    printf("input group name:\n");
    scanf("%s", body->msg_a);
    printf("input member name you want to ban:\n");
    scanf("%s", body->msg_b);
    strcpy(body->msg_c, account);

    send_fd(body, fd);

    free(body);
    return SUCCESS;
}

/*解开禁言 */
int unlock_ban(int fd, char *account)
{
    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = UNLOCKBAN;

    printf("input group name:\n");
    scanf("%s", body->msg_a);
    printf("input member name you want to unlock:\n");
    scanf("%s", body->msg_b);
    strcpy(body->msg_c, account);

    send_fd(body, fd);

    free(body);
    return SUCCESS;
}

/*解散群 */
int dissolve_group(int fd, char *account)
{
    char temp[SIZE];
    bzero(temp, sizeof(temp));

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    body->action = DISSOLVEGROUP;
    printf("input group name:\n");
    scanf("%s", body->msg_a);
    strcpy(body->msg_b, account);

    send_fd(body, fd);

    bzero(body, sizeof(struct message));
    recv_fd(body, fd);

    if (strcmp(body->msg_d, "ok") == 0)
    {
        printf("are you sure dissolve this group?:y/n\n");
        scanf("%s", temp);

        if (temp[0] == 'y') //再次确定删除
        {
            send_fd(body, fd);
            free(body);
            return SUCCESS;
        }

        if (temp[0] == 'n') //拒绝删除
        {
            strcpy(body->msg_d, "no");

            send_fd(body, fd);
            free(body);
            return FALSE;
        }
    }
    else if (strcmp(body->msg_d, "no") == 0) //身份验证失败
    {
        printf("fail\n");
        sleep(1);

        free(body);
        return FALSE;
    }
}
