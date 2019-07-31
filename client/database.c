#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "../header.h"
#include "client_header.h"

static int mark = 0;
static struct message temp;

/*打开数据库 */
void open_db(sqlite3 **ppdb)
{
	if (sqlite3_open("client.db", ppdb) != SQLITE_OK)
	{
		printf("sqlite3_open:%s\n", sqlite3_errmsg(*ppdb));
		exit(1);
	}
}

/*初始化数据库*/
void InitDataBase(void)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "create table if not exists mempwd (name text primary key,account text,passwd text);");
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	printf("initdatabase success...\n");
}

int sendinfo(void *par, int columnCount, char **columnValue, char **columnName)
{
	mark = 1;

	bzero(&temp, sizeof(struct message));

	strcpy(temp.msg_a, columnValue[0]);
	strcpy(temp.msg_d, columnValue[2]);

	return 0;
}

/*搜索用户是否记录过*/
int Search_user(struct message *body)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "select * from mempwd where name = '%s';", body->msg_a);
	ret = sqlite3_exec(ppdb, sql, sendinfo, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 0) //未曾记录过
	{
		return FALSE;
	}
	else if (mark == 1) //记录过密码
	{
		mark = 0;

		if (strcmp(temp.msg_d, body->msg_d) == 0) //密码存在且可用
		{
			return SUCCESS;
		}
		else //密码已被修改
			return MODIFY;
	}

	sqlite3_close(ppdb);
}

int sendinfo_ver1(void *par, int columnCount, char **columnValue, char **columnName)
{
	mark = 1;

	bzero(&temp, sizeof(struct message));

	strcpy(temp.msg_a, columnValue[0]);
	strcpy(temp.msg_c, columnValue[1]);
	strcpy(temp.msg_d, columnValue[2]);

	return 0;
}
/*搜索用户密码*/
void Search_pwd(struct message *body)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "select * from mempwd where name = '%s';", body->msg_a);
	ret = sqlite3_exec(ppdb, sql, sendinfo_ver1, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	if (mark == 1) //使用账户
	{
		mark = 0;
		strcpy(body->msg_c, temp.msg_c);
		strcpy(body->msg_d, temp.msg_d);
	}
	else //使用账号
	{
		sprintf(sql, "select * from mempwd where account = '%s';", body->msg_a);
		ret = sqlite3_exec(ppdb, sql, sendinfo_ver1, NULL, NULL);
		if (ret != SQLITE_OK)
		{
			printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
			exit(1);
		}

		if (mark == 1)
		{
			mark = 0;

			strcpy(body->msg_a, temp.msg_a);
			strcpy(body->msg_c, temp.msg_c);
			strcpy(body->msg_d, temp.msg_d);
		}
	}

	sqlite3_close(ppdb);

	printf("searchpwd success\n");
}
/*记住密码，将账号以及密码记入数据库*/
void Insert_user(struct message *body)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "insert into mempwd values ('%s','%s','%s');", body->msg_a, body->msg_c, body->msg_d);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	printf("remember pwd success...\n");
}

/*删除用户 */
void delete_user(struct message *body)
{
	sqlite3 *ppdb;
	int ret;

	open_db(&ppdb);

	char sql[SIZE];
	bzero(sql, sizeof(sql));

	sprintf(sql, "delete from mempwd where name = '%s';", body->msg_a);
	ret = sqlite3_exec(ppdb, sql, NULL, NULL, NULL);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec:%s\n", sqlite3_errmsg(ppdb));
		exit(1);
	}

	sqlite3_close(ppdb);

	printf("delete front pwd success...\n");
}
