#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "server_header.h"
#include "../header.h"

extern time_t timep;

/*链表初始化 */
void Init_list(Link *head)
{
    *head = NULL;

    time(&timep);
    printf("Init_list success...\t%s\n", ctime(&timep));
}

/*根据用户名查找fd */
int find_fd(char *str, Link head)
{
    if (head == NULL)
    {
        time(&timep);
        printf("empty list...\t%s\n", ctime(&timep));
        return FALSE;
    }

    Link temp = head;

    while (temp != NULL)
    {
        if (strcmp(temp->name, str) == 0)
        {
            time(&timep);
            printf("find fd success...\t%s\n", ctime(&timep));

            return temp->cfd;
        }

        temp = temp->next;
    }

    return FALSE;
}

/*根据账号查找fd */
int find_account_fd(char *str, Link head)
{
    if (head == NULL)
    {
        time(&timep);
        printf("empty list...\t%s\n", ctime(&timep));
        return FALSE;
    }

    Link temp = head;

    while (temp != NULL)
    {
        if (strcmp(temp->account, str) == 0)
        {
            return SUCCESS;
        }

        temp = temp->next;
    }

    return FALSE;
}

/*检查账号是否已登录 */
int check_account(struct message *body, Link head)
{
    if (find_fd(body->msg_a, head) != FALSE) //判断在线链表中是否存在用户名结点
    {
        return FALSE; //结点已存在
    }

    if (find_account_fd(body->msg_a, head) == SUCCESS)
    {
        return FALSE;
    }

    return SUCCESS;
}

/*在链表中插入登录人账号信息 */
void insert_user(Link *head, Link new)
{
    if ((*head) == NULL)
    {
        new->next = NULL;
        (*head) = new;
    }
    else
    {
        new->next = (*head)->next;
        (*head)->next = new;
    }

    time(&timep);
    printf("insert into list success...\t%s\n", ctime(&timep));
}

/*删除用户结点 */
int delete_user(Link *head, int cfd)
{
    char name[SIZE];
    bzero(name, sizeof(name));

    if ((*head) == NULL)
    {
        time(&timep);
        printf("empty list...\t%s\n", ctime(&timep));
        return FALSE;
    }

    Link temp = *head;

    if ((*head)->cfd == cfd)
    {
        strcpy(name, (*head)->name);

        *head = (*head)->next;
        free(temp);
        temp = NULL;

        time(&timep);
        printf("%s is offline...\t%s\n", name, ctime(&timep));
        return SUCCESS;
    }
    else
    {
        Link ptr = temp;
        temp = temp->next;

        while (temp != NULL)
        {
            if (temp->cfd == cfd)
            {
                strcpy(name, temp->name);

                ptr->next = temp->next;
                free(temp);
                temp = NULL;

                time(&timep);
                printf("%s is offline...\t%s\n", name, ctime(&timep));
                return SUCCESS;
            }

            ptr = temp;
            temp = temp->next;
        }
    }

    return FALSE;
}

/*查找在线用户 */
void show_user(int fd, Link head)
{
    if (head == NULL)
    {
        time(&timep);
        printf("empty list...\t%s\n", ctime(&timep));
        exit(1);
    }

    struct message *body = (struct message *)malloc(sizeof(struct message));
    bzero(body, sizeof(body));

    Link temp = head;

    body->action = EMPTYMSG;
    send_fd(body, fd); //发送一个空结构指针来解决客户端阻塞接收的问题

    while (temp != NULL)
    {
        bzero(body, sizeof(struct message));

        strcpy(body->msg_a, temp->name);
        strcpy(body->msg_c, temp->account);

        send_fd(body, fd);

        temp = temp->next;

        time(&timep);
        printf("send online list...\t%s\n", ctime(&timep));
    }

    bzero(body, sizeof(struct message));
    strcpy(body->msg_a, "end");

    send_fd(body, fd);
}

/*私聊 */
int private_chat(struct message *body, Link head, int cfd)
{
    if (exist_user(body, cfd) == FALSE) //检查数据库中是否有该目标用户
        return FALSE;

    int fd = find_fd(body->msg_a, head);
    if (fd == FALSE) //不在线
    {
        time(&timep);
        printf("this user is offline...\t%s\n", ctime(&timep));

        insert_offline_msg(body);

        return SUCCESS;
    }

    send_fd(body, fd);

    time(&timep);
    printf("private chat success...\t%s\n", ctime(&timep));

    return SUCCESS;
}

/*向所有在线用户发消息 */
int public_chat(struct message *body, Link head)
{
    if (head == NULL)
    {
        time(&timep);
        printf("empty list\t%s\n", ctime(&timep));
        return FALSE;
    }

    Link temp = head;

    while (temp != NULL)
    {
        send_fd(body, temp->cfd);
        temp = temp->next;
    }

    time(&timep);
    printf("public chat success...\t%s\n", ctime(&timep));
}
