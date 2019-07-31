#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../header.h"
#include "client_header.h"

/*获得时间 */
void getTime(char *pTime)
{
	time_t nSeconds;
	struct tm *pTM;

	time(&nSeconds);
	pTM = localtime(&nSeconds);

	sprintf(pTime, "%04d-%02d-%02d\t%02d:%02d:%02d", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	printf("\t%s\n", pTime);
}

/*发送 */
void send_fd(struct message *body, int fd)
{
	if (send(fd, body, sizeof(struct message), 0) < 0)
	{
		perror("send");
		exit(1);
	}
}

/*接收 */
void recv_fd(struct message *body, int fd)
{
	int ret;

	if ((ret = recv(fd, body, sizeof(struct message), 0)) < 0)
	{
		perror("recv");
		exit(1);
	}

	if (0 == ret) /*验证接收为0就退出 */
	{
		printf("%d is close\n", fd);
		exit(1);
	}
}

/*检查密码*/
int checkpwd(char *str)
{
	int i = 3;
	char passwd[SIZE];

	while (i != 0)
	{
		printf("input passwd:\n");

		system("stty -echo");
		scanf("%s", passwd);
		system("stty echo");

		if (strcmp(passwd, str) == 0)
		{
			printf("pwd correct\n");
			break;
		}
		else
		{
			printf("pwd error\n");
			i--;
		}
	}

	if (0 == i)
	{
		return FALSE;
	}

	return SUCCESS;
}

/*登录成功后返回消息登录上线 */
void logon_success(int fd, struct message *body)
{
	body->action = LOGIN_SUCCESS;
	/*if (send(fd, body, sizeof(struct message), 0) < 0)
	{
		perror("send LOGIN");
		exit(1);
	}*/
	send_fd(body, fd);
}

/*验证登录用 */
int log_on(int fd, char *name, char *account)
{
	char select[SIZE];
	int mark = FALSE;
	int book;

	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	printf("Please inpuit name/account:\n");
	scanf("%s", body->msg_a);

	body->action = LOGIN;
	send_fd(body, fd);

	bzero(body, sizeof(struct message));
	recv_fd(body, fd);

	if (strcmp(body->msg_b, "not exist") == 0) //账号不存在
	{
		printf("no this account!\n");
		sleep(1);

		return FALSE;
	}

	if (body->action == ONLINE) //验证用户是否已在线
	{
		printf("user is already online!\n");
		sleep(2);

		return FALSE;
	}

	if (checkpwd(body->msg_d) == SUCCESS) //密码验证成功
	{
		if ((book = Search_user(body)) != SUCCESS) //如果未曾记录过密码或密码已修改让其选择是否记住密码或修改密码
		{
			printf("remember pwd?：y/n\n");
			scanf("%s", select);

			if (select[0] == 'y')
			{
				if (book == FALSE) //如果是未记录过的就插入
				{
					Insert_user(body);
				}
				if (book == MODIFY)
				{
					delete_user(body);
					Insert_user(body);
				}
			}
		}

		strcpy(name, body->msg_a);
		strcpy(account, body->msg_c);

		mark = SUCCESS;
		logon_success(fd, body); //发送密码验证成功
	}

	free(body);
	return mark;
}

/*注册用户 */
int reg_user(int fd)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	printf("input reg name:\n");
	scanf("%s", body->msg_a);

	body->action = REG;
	send_fd(body, fd); //发送用户名检查用户名是否可用

	bzero(body, sizeof(struct message));
	recv_fd(body, fd);

	if (strcmp(body->msg_b, "not exist") == 0) //账号名可使用
	{
		printf("user name is available\n");

		printf("input idnum:\n");
		bzero(body->msg_b, sizeof(body->msg_b));
		scanf("%s", body->msg_b);
		printf("input password:\n");
		scanf("%s", body->msg_d);

		send_fd(body, fd); //发送注册密码信息

		bzero(body->msg_b, sizeof(body->msg_b));
		recv_fd(body, fd); //接收账号

		printf("reg success!\nyour account:%s\n", body->msg_c);

		sleep(3);
		free(body);
		return SUCCESS;
	}
	else if (strcmp(body->msg_b, "exist") == 0) //账号名不可用
	{
		printf("user is in used\n");
		sleep(1);

		free(body);
		return FALSE;
	}
}

/*自动登录 */
int auto_logon(int fd, char *name, char *account)
{
	int mark = FALSE;

	struct message temp;
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	printf("input name/account:\n");
	scanf("%s", body->msg_a);

	body->action = AUTOLOG;

	send_fd(body, fd);

	bzero(body, sizeof(struct message));
	recv_fd(body, fd);

	if (strcmp(body->msg_b, "not exist") == 0) //账号不存在
	{
		printf("no this account!\n");
		sleep(1);

		return FALSE;
	}

	if (body->action == ONLINE) //验证用户是否已在线
	{
		printf("user is already online!\n");
		sleep(2);

		return FALSE;
	}

	strcpy(temp.msg_a, body->msg_a);
	Search_pwd(&temp);

	if (strcmp(temp.msg_d, body->msg_d) == 0)
	{
		mark = SUCCESS;
		strcpy(name, body->msg_a);
		strcpy(account, body->msg_c);

		logon_success(fd, body); //发送密码验证成功
	}
	else
	{
		mark = FALSE;

		printf("password has been modified\n");
		sleep(1);
	}

	free(body);
	return mark;
}

/*找回密码 */
void find_pwd(int fd)
{
	char idnum[SIZE];
	bzero(idnum, sizeof(idnum));

	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	printf("input lost name:\n");
	scanf("%s", body->msg_a);
	body->action = FINDLOST;

	send_fd(body, fd);

	bzero(body, sizeof(struct message));

	recv_fd(body, fd);

	printf("input idnum:\n");
	scanf("%s", idnum);

	if (strcmp(idnum, body->msg_b) == 0)
	{
		printf("password：%s\ndon't forget again!\n", body->msg_d);
	}

	sleep(4);

	free(body);
}

/*注销用户 */
int drop_user(int fd)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	printf("input delete name:\n");
	scanf("%s", body->msg_a);
	printf("input password to prove you:\n");
	scanf("%s", body->msg_d);
	body->action = DROPUSER;

	send_fd(body, fd);

	bzero(body, sizeof(struct message));

	recv_fd(body, fd);

	if (strcmp(body->msg_b, "not exist") == 0)
	{
		printf("this account is not exist\n");
		sleep(1);
		return SUCCESS;
	}
	else if (strcmp(body->msg_b, "delete success") == 0)
	{
		printf("delete success\n");
		sleep(1);
		return SUCCESS;
	}
	else
	{
		printf("password error\n");
		sleep(1);
		return FALSE;
	}
}

/*显示在线用户 */
void showuser(int fd)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	body->action = SHOWMEMBER;
	send_fd(body, fd);

	while (1) //循环接收信息
	{
		bzero(body, sizeof(struct message));

		recv_fd(body, fd);

		if (strcmp(body->msg_a, "end") == 0) //终止接收
			break;

		printf("\tname:%s\taccount:%s\n", body->msg_a, body->msg_c);
	}
	sleep(4);

	free(body);
}

/*私聊 */
void private_chat(int fd, char *name)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	char now[SIZE];
	char temp[SIZE];

	body->action = PRIVATECHAT;
	strcpy(body->msg_c, name);

	printf("input sendto name:\n");
	scanf("%s", body->msg_a);

	while (1)
	{
		bzero(temp, sizeof(temp));
		printf("input send message:\n");

		bzero(now, sizeof(now));
		getTime(now); //获得并显示时间
		strcat(now, "\n");

		scanf("%s", temp); //获得消息

		if (strcmp(temp, "exit") == 0) //输入exit退出聊天
			break;

		strcat(now, temp);
		strcpy(body->msg_b, now);

		send_fd(body, fd);
	}

	free(body);
}

/*给所有在线人发送消息 */
void public_chat(int fd, char *name)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	body->action = PUBLICCHAT;
	strcpy(body->msg_c, name);

	printf("input send message:\n");
	scanf("%s", body->msg_b);

	send_fd(body, fd);

	free(body);
}

/*修改密码 */
void modify_pwd(int fd, char *name)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	body->action = MODIFYPWD;
	strcpy(body->msg_a, name);

	printf("input wanted password:\n");
	scanf("%s", body->msg_d);

	send_fd(body, fd); //发送目的密码

	bzero(body, sizeof(struct message));

	recv_fd(body, fd); //接收确认消息

	if (body->action == MODIFYPWD)
	{
		printf("modify password success, return login interface...\n");
		sleep(1);
	}

	free(body);
}

/*返回登录界面 */
int set_offline(int fd, char *name)
{
	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	body->action = OFFLINE;
	strcpy(body->msg_a, name);

	send_fd(body, fd);
	free(body);
	return SUCCESS;
}

