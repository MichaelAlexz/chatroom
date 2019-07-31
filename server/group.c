#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include "../header.h"
#include "server_header.h"

extern time_t timep;
static struct message temp;
static int mark = 0;
static sqlite3 *other = NULL;
Link head_temp = NULL;

static int sendinfo_ver6(void *, int, char **, char **);

/*建群 */
int new_group(struct message *body, int cfd)
{
    char *id = body->msg_c;
    GetId(&id); //生成群账号

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];
    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s';", body->msg_a);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //群名重复
    {
        mark = 0;

        time(&timep);
        printf("group %s create fail...\t%s\n", body->msg_a, ctime(&timep));

        strcpy(body->msg_b, "fail");
        send_fd(body, cfd);

        sqlite3_close(ppdb);

        return FALSE;
    }
    else
    {
        bzero(sql, sizeof(sql));
        sprintf(sql, "insert into groupmsg values ('%s','%s','%s','%s',1,0);", body->msg_a, body->msg_c, body->msg_b, body->msg_d);
        ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        sqlite3_close(ppdb);

        time(&timep);
        printf("group %s create success...\t%s\n", body->msg_a, ctime(&timep));

        strcpy(body->msg_b, "success");
        send_fd(body, cfd);
        
        return SUCCESS;
    }
}

static int sendinfo_ver1(void *par, int columnCount, char **columnValue, char **columnName)
{
    int cfd = *(int *)par;

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    strcpy(body->msg_a, columnValue[0]);
    strcpy(body->msg_c, columnValue[1]);

    send_fd(body, cfd);

    free(body);

    return 0;
}
/*显示已加入的群 */
int show_groups(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where useraccount = '%s';", body->msg_a);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver1, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    sqlite3_close(ppdb);

    bzero(body, sizeof(struct message));
    strcpy(body->msg_a, "end");
    send_fd(body, cfd);

    time(&timep);
    printf("showgroups success...\t%s\n", ctime(&timep));

    return SUCCESS;
}

static int sendinfo_ver2(void *par, int columnCount, char **columnValue, char **columnName)
{
    mark = 1;

    bzero(&temp, sizeof(struct message));
    strcpy(temp.msg_a, columnValue[0]);

    return 0;
}

//显示我管理的群
int show_mananger_group(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where useraccount = '%s' and mark = 1 ;", body->msg_a);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver1, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    sqlite3_close(ppdb);

    bzero(body, sizeof(struct message));
    strcpy(body->msg_a, "end");
    send_fd(body, cfd);

    time(&timep);
    printf("show manager groups success...\t%s\n", ctime(&timep));

    return SUCCESS;
}

/*加群 */
int insert_groupmsg(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select groupname from groupmsg where groupaccount = '%s';", body->msg_b);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver2, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //确定有该账号对应的群后再插入信息
    {
        mark = 0;

        strcpy(body->msg_a, temp.msg_a);
        bzero(sql, sizeof(sql));
        sprintf(sql, "insert into groupmsg values ('%s','%s','%s','%s',0,0);", body->msg_a, body->msg_b, body->msg_c, body->msg_d); //普通成员，权限为0,未被禁言
        ret = sqlite3_exec(ppdb, sql, sendinfo_ver2, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        sqlite3_close(ppdb);

        body->action = EMPTYMSG; //暂停接收线程
        send_fd(body, cfd);

        send_fd(body, cfd); //发送验证消息

        time(&timep);
        printf("%s insert into %s success\t%s\n", body->msg_c, body->msg_a, ctime(&timep));

        return SUCCESS;
    }
    else
    {
        sqlite3_close(ppdb);

        body->action = EMPTYMSG; //暂停接收线程
        send_fd(body, cfd);

        send_fd(body, cfd); //发送验证消息

        time(&timep);
        printf("%s insert into groupmsg failed\t%s\n", body->msg_c, ctime(&timep));

        return FALSE;
    }
}

/*退群 */
int exit_group(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "delete from groupmsg where username = '%s' and groupaccount = '%s';", body->msg_c, body->msg_b);
    ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    sqlite3_close(ppdb);

    time(&timep);
    printf("%s exit group...\t%s\n", body->msg_c, ctime(&timep));

    return SUCCESS;
}

static int sendinfo_ver4(void *par, int columnCount, char **columnValue, char **columnName) //发群员
{
    int cfd = *(int *)par;

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    if (strcmp(columnValue[5], "1") == 0) //添加禁言标志
    {
        strcpy(body->msg_a, columnValue[2]);
        strcpy(body->msg_b, columnValue[3]);
        strcat(body->msg_a, "*");

        send_fd(body, cfd);

        free(body);
        return 0;
    }

    strcpy(body->msg_a, columnValue[2]);
    strcpy(body->msg_b, columnValue[3]);

    send_fd(body, cfd);

    free(body);
    return 0;
}

static int sendinfo_ver3(void *par, int columnCount, char **columnValue, char **columnName) //发群
{
    mark = 1;

    int cfd = *(int *)par;
    char sql[SIZE];
    int ret;

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(struct message));

    strcpy(body->msg_a, columnValue[0]);
    strcat(body->msg_a, "\t");
    strcpy(body->msg_b, columnValue[1]);

    send_fd(body, cfd);

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s';", columnValue[0]);
    ret = sqlite3_exec(other, sql, sendinfo_ver4, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec2:%s\n", sqlite3_errmsg(other));
        exit(1);
    }

    free(body);

    return 0;
}
/*显示所有群员 */
int show_group_mem(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    other = ppdb; //全局变量

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where useraccount = '%s';", body->msg_a);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver3, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark)
    {
        mark = 0;
        strcpy(body->msg_a, "end");

        send_fd(body, cfd);

        sqlite3_close(ppdb);
        other = NULL;
        return SUCCESS;
    }
    else
    {
        strcpy(body->msg_a, "fail");

        send_fd(body, cfd);

        sqlite3_close(ppdb);

        return FALSE;
    }
}

/*显示所有我管理的群成员 */
int show_mangroup_mem(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    other = ppdb; //全局变量

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where useraccount = '%s' and mark = 1;", body->msg_a);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver3, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark)
    {
        mark = 0;
        strcpy(body->msg_a, "end");

        send_fd(body, cfd);

        sqlite3_close(ppdb);
        other = NULL;
        return SUCCESS;
    }
    else
    {
        strcpy(body->msg_a, "fail");

        send_fd(body, cfd);

        sqlite3_close(ppdb);

        return FALSE;
    }
}

static int sendinfo_ver6(void *par, int columnCount, char **columnValue, char **columnName) //专用于查询结果是否存在
{
    mark = 1;

    return 0;
}

static int sendinfo_ver5(void *par, int columnCount, char **columnValue, char **columnName) //发群员
{
    char sql[SIZE];
    int ret;

    int cfd = *(int *)par;
    char username[SIZE];
    bzero(username, sizeof(username));

    strcpy(username, columnValue[2]);
    strcpy(temp.msg_a, username);

    int fd = find_fd(username, head_temp);
    if (FALSE == fd)
    {
        time(&timep);
        printf("group user %s is offline...\t%s\n", username, ctime(&timep));

        bzero(sql, sizeof(sql));

        sprintf(sql, "insert into offline values ('%s','%s','%s');", temp.msg_c, temp.msg_a, temp.msg_b);
        ret = sqlite3_exec(other, sql, NULL, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec:%s\n", sqlite3_errmsg(other));
            exit(1);
        }

        time(&timep);
        printf("offline message to %s insert success...\t%s\n", username, ctime(&timep));

        return 0;
    }

    send_fd(&temp, fd);

    time(&timep);
    printf("send group message to %s success...\t%s\n", username, ctime(&timep));

    return 0;
}
/*发送群消息 */
int send_groupmsg(struct message *body, int cfd, Link head)
{
    head_temp = head; //给全局变量赋值
    bzero(&temp, sizeof(struct message));
    strcpy(temp.msg_c, body->msg_c);
    strcpy(temp.msg_b, body->msg_b);

    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    other = ppdb;

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s' and username = '%s' and banmark = 0;", body->msg_a, body->msg_d);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, &cfd, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //发送人未被禁言
    {
        bzero(sql, sizeof(sql));
        sprintf(sql, "select * from groupmsg where groupname = '%s';", body->msg_a);
        ret = sqlite3_exec(ppdb, sql, sendinfo_ver5, &cfd, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        sqlite3_close(ppdb);

        other = NULL;

        time(&timep);
        printf("send group message success...\t%s\n", ctime(&timep));
        return SUCCESS;
    }
    else
    {
        bzero(body, sizeof(struct message));

        strcpy(body->msg_c, "server");
        strcpy(body->msg_b, "you have been banned");
        send_fd(body, cfd);

        sqlite3_close(ppdb);

        other = NULL;

        time(&timep);
        printf("send group message fail...\t%s\n", ctime(&timep));
        return FALSE;
    }
}

//踢人
int delete_groupmem(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s' and useraccount = '%s' and mark = 1;", body->msg_a, body->msg_c);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //确定管理员身份
    {
        mark = 0;

        bzero(sql, sizeof(sql));
        sprintf(sql, "select * from groupmsg where groupname = '%s' and username = '%s' and mark = 0;", body->msg_a, body->msg_b);
        ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        if (1 == mark) //确定成员身份
        {
            mark = 0;

            bzero(sql, sizeof(sql));
            sprintf(sql, "delete from groupmsg where username = '%s' and groupname = '%s';", body->msg_b, body->msg_a);
            ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
                exit(1);
            }

            sqlite3_close(ppdb);

            time(&timep);
            printf("delete %s from %s success\t%s\n", body->msg_b, body->msg_a, ctime(&timep));
            return SUCCESS;
        }
        else
        {
            sqlite3_close(ppdb);

            time(&timep);
            printf("delete fail\t%s\n", ctime(&timep));
            return FALSE;
        }
    }
    else
    {
        sqlite3_close(ppdb);

        time(&timep);
        printf("delete fail\t%s\n", ctime(&timep));
        return FALSE;
    }
}

//禁言
int ban_groupuser(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s' and useraccount = '%s' and mark = 1;", body->msg_a, body->msg_c);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //确定管理员身份正确
    {
        mark = 0;

        bzero(sql, sizeof(sql));
        sprintf(sql, "select * from groupmsg where groupname = '%s' and username = '%s' and mark = 0;", body->msg_a, body->msg_b);
        ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        if (1 == mark) //确定成员身份正确
        {
            mark = 0;

            bzero(sql, sizeof(sql));
            sprintf(sql, "update groupmsg set banmark = 1 where groupname = '%s' and username = '%s';", body->msg_a, body->msg_b);
            ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
                exit(1);
            }

            sqlite3_close(ppdb);

            time(&timep);
            printf("ban %s success\t%s\n", body->msg_b, ctime(&timep));
            return SUCCESS;
        }
        else
        {
            sqlite3_close(ppdb);

            time(&timep);
            printf("ban fail\t%s\n", ctime(&timep));
            return FALSE;
        }
    }
    else
    {
        sqlite3_close(ppdb);

        time(&timep);
        printf("ban fail\t%s\n", ctime(&timep));
        return FALSE;
    }
}

//解禁
int unlock_ban(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s' and useraccount = '%s' and mark = 1;", body->msg_a, body->msg_c);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark) //确定管理员身份正确
    {
        mark = 0;

        bzero(sql, sizeof(sql));
        sprintf(sql, "select * from groupmsg where groupname = '%s' and username = '%s' and mark = 0;", body->msg_a, body->msg_b);
        ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
            exit(1);
        }

        if (1 == mark) //确定成员身份正确
        {
            mark = 0;

            bzero(sql, sizeof(sql));
            sprintf(sql, "update groupmsg set banmark = 0 where groupname = '%s' and username = '%s';", body->msg_a, body->msg_b);
            ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
                exit(1);
            }

            sqlite3_close(ppdb);

            time(&timep);
            printf("unlock ban %s success\t%s\n", body->msg_b, ctime(&timep));
            return SUCCESS;
        }
        else
        {
            sqlite3_close(ppdb);

            time(&timep);
            printf("unlock ban fail\t%s\n", ctime(&timep));
            return FALSE;
        }
    }
    else
    {
        sqlite3_close(ppdb);

        time(&timep);
        printf("unlock ban fail\t%s\n", ctime(&timep));
        return FALSE;
    }
}

//解散群
int dissolve_group(struct message *body, int cfd)
{
    sqlite3 *ppdb;
    int ret;

    open_db(&ppdb);

    body->action = EMPTYMSG; //暂停接收线程
    send_fd(body, cfd);

    char sql[SIZE];

    bzero(sql, sizeof(sql));
    sprintf(sql, "select * from groupmsg where groupname = '%s' and useraccount = '%s' and mark = 1;", body->msg_a, body->msg_b);
    ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
        exit(1);
    }

    if (1 == mark)
    {
        mark = 0;

        strcpy(body->msg_d, "ok");
        send_fd(body, cfd);

        bzero(body, sizeof(struct message));
        recv_fd(body, cfd);

        if (strcmp(body->msg_d, "ok") == 0)
        {
            bzero(sql, sizeof(sql));
            sprintf(sql, "delete from groupmsg where groupname = '%s' ;", body->msg_a);
            ret = sqlite3_exec(ppdb, sql, sendinfo_ver6, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                printf("sqlite3_exec1:%s\n", sqlite3_errmsg(ppdb));
                exit(1);
            }

            sqlite3_close(ppdb);

            time(&timep);
            printf("dissolve group %s success...\t%s\n", body->msg_a, ctime(&timep));

            return SUCCESS;
        }
        else if (strcmp(body->msg_d, "no") == 0)
        {
            sqlite3_close(ppdb);

            time(&timep);
            printf("Refuse to dissolve group %s...\t%s\n", body->msg_a, ctime(&timep));

            return SUCCESS;
        }
    }
    else
    {
        strcpy(body->msg_d, "no");
        send_fd(body, cfd);

        sqlite3_close(ppdb);

        time(&timep);
        printf("dissolve group %s fail...\t%s\n", body->msg_a, ctime(&timep));

        return FALSE;
    }
}