#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "../header.h"
#include "server_header.h"

extern time_t timep;
static int mark = 0;
static struct message temp;

/*打开数据库 */
void open_db(sqlite3 **ppdb)
{
	if (sqlite3_open("server.db", ppdb) != SQLITE_OK)
	{
		printf("sqlite3_open:%s\n", sqlite3_errmsg(*ppdb));
		exit(1);
	}
}
/*
初始化数据库及创建表
*/
void InitDataBase(void)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "create table if not exists usermsg (name text primary key,idnum text,account text,passwd text);"); //创建用户信息表
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	bzero(sql, sizeof(sql));
	sprintf(sql, "create table if not exists offline (sender text, recver text, message text);"); //创建下线消息表
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	bzero(sql, sizeof(sql));
	sprintf(sql, "create table if not exists groupmsg (groupname text, groupaccount text, username text, useraccount text, mark int, banmark int);"); //创建群信息表
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	time(&timep);
	printf("initdatabase success...\t%s\n", ctime(&timep));
}

/*在表中插入注册人信息及账号*/
void Insert_user(struct message *body, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "insert into usermsg values ('%s','%s','%s','%s');", body->msg_a, body->msg_b, body->msg_c, body->msg_d);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	send_fd(body, cfd);

	time(&timep);
	printf("%s insert success...\t%s\n", body->msg_a, ctime(&timep));
}

/* 用于查找账号所有信息的回调函数*/
static int sendinfo(void *par, int columnCount, char **columnValue, char **columnName)
{
	mark = 1;

	int cfd = *(int *)par;

	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	strcpy(body->msg_a, columnValue[0]);
	strcpy(body->msg_b, columnValue[1]);
	strcpy(body->msg_c, columnValue[2]);
	strcpy(body->msg_d, columnValue[3]);

	send_fd(body, cfd);

	time(&timep);
	printf("sendinfo success...\t%s\n", ctime(&timep));

	return 0;
}

/*搜查对应账号信息*/
void Checkpwd(char *ptr, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	time(&timep);
	printf("user:%s\t%s\n", ptr, ctime(&timep));

	sprintf(sql, "select * from usermsg where name = '%s';", ptr);
	ret = sqlite3_exec(ppdb, sql, sendinfo, &cfd, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 1)
	{
		mark = 0;

		time(&timep);
		printf("使用账户名\t%s\n", ctime(&timep));
	}
	else
	{
		sprintf(sql, "select * from usermsg where account = '%s';", ptr);
		ret = sqlite3_exec(ppdb, sql, sendinfo, &cfd, NULL);
		if (ret != SQLITE_OK)
		{
			printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
			exit(1);
		}

		if (mark == 1)
		{
			mark = 0;

			time(&timep);
			printf("使用账号\t%s\n", ctime(&timep));
		}
	}

	sqlite3_close(ppdb);

	time(&timep);
	printf("checkpwd success\t%s\n", ctime(&timep));
}

static int sendinfo_ver1(void *par, int columnCount, char **columnValue, char **columnName)
{
	int cfd = *(int *)par;

	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	strcpy(body->msg_a, columnValue[0]);
	strcpy(body->msg_b, columnValue[1]);
	strcpy(body->msg_c, columnValue[2]);
	strcpy(body->msg_d, columnValue[3]);

	send_fd(body, cfd);

	return 0;
}
/*
通过账号名搜查对应账号信息
*/
void Findpwd(char *ptr, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	printf("user:%s\n", ptr);

	sprintf(sql, "select * from usermsg where name = '%s';", ptr);
	ret = sqlite3_exec(ppdb, sql, sendinfo_ver1, &cfd, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	time(&timep);
	printf("checkpwd success...\t%s\n", ctime(&timep));
}

static int sendinfo_ver5(void *par, int columnCount, char **columnValue, char **columnName)
{
	printf("%s", columnValue[3]);

	return 0;
}

/*修改密码 */
void modify_pwd(struct message *body, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	time(&timep);
	printf("goal pwd:%s\t%s\n", body->msg_d, ctime(&timep));

	sprintf(sql, "update usermsg set passwd = '%s' where name = '%s';", body->msg_d, body->msg_a);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("modify pwd:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	time(&timep);
	printf("modify pwd success...\t%s\n", ctime(&timep));

	body->action = EMPTYMSG;
	send_fd(body, cfd); //发送一个空结构指针来解决客户端阻塞接收的问题

	body->action = MODIFYPWD; //发送修改成功的消息
	send_fd(body, cfd);
}

static int sendinfo_ver4(void *par, int columnCount, char **columnValue, char **columnName)
{
	mark = 1;

	return 0;
}

/*检查账户是否存在 */
int exist_user(struct message *body, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "select * from usermsg where name = '%s';", body->msg_a); //检查姓名
	ret = sqlite3_exec(ppdb, sql, sendinfo_ver4, &cfd, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 1)
	{
		mark = 0;
		time(&timep);
		printf("存在账户名\t%s\n", ctime(&timep));
		sqlite3_close(ppdb);
		return SUCCESS;
	}

	bzero(sql, sizeof(sql));

	sprintf(sql, "select * from usermsg where account = '%s';", body->msg_a); //检查账户号
	ret = sqlite3_exec(ppdb, sql, sendinfo_ver4, &cfd, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 1)
	{
		mark = 0;
		time(&timep);
		printf("存在账号\t%s\n", ctime(&timep));
		sqlite3_close(ppdb);
		return SUCCESS;
	}

	if (mark == 0)
	{
		strcpy(body->msg_b, "not exist");
		send_fd(body, cfd);

		time(&timep);
		printf("不存在对应账号\t%s\n", ctime(&timep));
		return FALSE;
	}
}

/*注册账号 */
int reg_user(struct message *body, int cfd)
{
	if (exist_user(body, cfd) == SUCCESS) //账号存在直接退出
	{
		strcpy(body->msg_b, "exist");
		send_fd(body, cfd);

		time(&timep);
		printf("user name is in used\t%s\n", ctime(&timep));

		return FALSE;
	}

	bzero(body, sizeof(struct message));
	recv_fd(body, cfd);

	char *temp = body->msg_c;
	GetId(&temp);
	Insert_user(body, cfd); //插入用户信息

	time(&timep);
	printf("reg user success...\t%s\n", ctime(&timep));
	return SUCCESS;
}

static int sendinfo_ver2(void *par, int columnCount, char **columnValue, char **columnName)
{
	bzero(&temp, sizeof(struct message));
	strcpy(temp.msg_d, columnValue[3]);

	return 0;
}

/*删除数据库中用户 */
int drop_user(struct message *body, int cfd)
{
	if (exist_user(body, cfd) != SUCCESS) //账号不存在
	{
		return SUCCESS; //会发送不存在结果
	}

	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));
	sprintf(sql, "select * from usermsg where name = '%s';", body->msg_a);
	ret = sqlite3_exec(ppdb, sql, sendinfo_ver2, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (strcmp(temp.msg_d, body->msg_d) == 0) //验证通过进行删除操作
	{
		bzero(sql, sizeof(sql));
		sprintf(sql, "delete from usermsg where name = '%s';", body->msg_a);
		ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		{
			printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
			exit(1);
		}

		strcpy(body->msg_b, "delete success"); //发送删除成功消息
		send_fd(body, cfd);

		sqlite3_close(ppdb);

		time(&timep);
		printf("delete %s from usermsg success...\t%s\n", body->msg_a, ctime(&timep));

		return SUCCESS;
	}
	else //密码验证错误
	{
		strcpy(body->msg_b, "delete failed");
		send_fd(body, cfd);

		sqlite3_close(ppdb);

		time(&timep);
		printf("delete %s from usermsg failed...\t%s\n", body->msg_a, ctime(&timep));

		return FALSE;
	}
}

/*插入离线消息 */
void insert_offline_msg(struct message *body)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "insert into offline values ('%s','%s','%s');", body->msg_c, body->msg_a, body->msg_b);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	time(&timep);
	printf("to %s offline message insert success...\t%s\n", body->msg_a, ctime(&timep));
}

static int sendinfo_offline(void *par, int columnCount, char **columnValue, char **columnName)
{
	mark = 1;

	int cfd = *(int *)par;

	struct message *body = (struct message *)malloc(sizeof(struct message));
	bzero(body, sizeof(struct message));

	strcpy(body->msg_c, columnValue[0]);
	strcpy(body->msg_b, columnValue[2]);

	send_fd(body, cfd);

	time(&timep);
	printf("send offline message to %s success\t%s\n", columnValue[1], ctime(&timep));

	free(body);
	return 0;
}

//验证与发送离线消息
int send_offline_msg(struct message *body, int cfd)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "select * from offline where recver = '%s';", body->msg_a);
	ret = sqlite3_exec(ppdb, sql, sendinfo_offline, &cfd, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 1) //确认发送了线下消息后删除表中数据
	{
		mark = 0;

		bzero(sql, sizeof(sql));
		sprintf(sql, "delete from offline where recver = '%s';", body->msg_a);
		ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
		if (ret != SQLITE_OK)
		{
			printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
			exit(1);
		}

		time(&timep);
		printf("delete offline message success\t%s\n", ctime(&timep));
	}

	sqlite3_close(ppdb);
}

