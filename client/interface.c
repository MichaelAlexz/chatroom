#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../header.h"
#include "client_header.h"

/*线程挂起与继续 */
static pthread_mutex_t mutex;
static pthread_cond_t cond;
static int flag = 0;
static int pthread_death = 0;

void srpthread_init()
{
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
}

void srpthread_suspend() //线程挂起
{
	pthread_mutex_lock(&mutex);
	flag--;
	pthread_mutex_unlock(&mutex);
}

void srpthread_resume() //线程继续
{
	pthread_mutex_lock(&mutex);
	flag++;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

/*登录界面*/
void Login(void)
{
	system("clear");

	printf("\t----------login------------\t\n");
	printf("\t-----1.登录-----------------\t\n");
	printf("\t-----2.注册-----------------\t\n");
	printf("\t-----3.自动登录--------------\t\n");
	printf("\t-----4.找回密码--------------\t\n");
	printf("\t-----5.注销账号--------------\t\n");
	printf("\t-----0.退出-----------------\t\n");
	printf("input cmd:\n");
}

/*用户界面*/
void Using(char *str, char *ptr)
{
	system("clear");

	printf("\t---user:%s---------- \t\n", str);
	printf("\t---account:%s----------\t\n", ptr);
	printf("\t---------chat room-----------------\t\n");
	printf("\t------1.显示在线用户-----------------\t\n");
	printf("\t------2.单发消息---------------------\t\n");
	printf("\t------3.向所有在线用户发送消息---------\t\n");
	printf("\t------4.群功能-----------------------\t\n");
	printf("\t------5.发文件-----------------------\t\n");
	printf("\t         ---                        \t\n");
	printf("\t         ---                        \t\n");
	printf("\t----- 8.修改密码---------------------\t\n");
	printf("\t------9.返回登录---------------------\t\n");
	printf("\t------0.退出------------------------\t\n");
	printf("\t       其它键：刷新                   \t\n");
	printf("input cmd:\n");
}

/*群功能界面 */
void group_interface(char *str, char *ptr)
{
	system("clear");

	printf("\t---user:%s---------- \t\n", str);
	printf("\t---account:%s----------\t\n", ptr);
	printf("\t----------------group---------------\t\n");
	printf("\t-------1.显示我加入的群----------------\t\n");
	printf("\t-------2.发送群消息-------------------\t\n");
	printf("\t-------3.建群------------------------\t\n");
	printf("\t-------4.显示我加入的群成员-------------\t\n");
	printf("\t-------5.加群------------------------\t\n");
	printf("\t-------6.退群------------------------\t\n");
	printf("\t-------7.管理我的群-------------------\t\n");
	printf("\t-------0.返回上一层-------------------\t\n");
	printf("\t       其它键：刷新                   \t\n");
	printf("input cmd:\n");
}

/*群管理界面 */
void group_manager_inter(char *str, char *ptr)
{
	system("clear");

	printf("\t---user:%s---------- \t\n", str);
	printf("\t---account:%s----------\t\n", ptr);
	printf("\t----------group-manager-------------\t\n");
	printf("\t-------1.显示我管理的群----------------\t\n");
	printf("\t-------2.显示我的群成员----------------\t\n");
	printf("\t-------3.踢人------------------------\t\n");
	printf("\t-------4.禁言------------------------\t\n");
	printf("\t-------5.解开禁言---------------------\t\n");
	printf("\t-------6.解散群聊---------------------\t\n");
	printf("\t-------0.返回上一层-------------------\t\n");
	printf("\t       其它键：刷新                   \t\n");
	printf("input cmd:\n");
}

/*群管理功能 */
int group_manager(int fd, char *name, char *account)
{
	char choice[SIZE];
	bzero(choice, sizeof(choice));

	while (1)
	{
		group_manager_inter(name, account);
		scanf("%s", choice);
		switch (choice[0])
		{
		case '1':
			srpthread_suspend();
			show_mananger_group(fd, account);
			srpthread_resume();
			break;

		case '2':
			srpthread_suspend();
			show_mangroup_mem(fd, account);
			srpthread_resume();
			break;

		case '3':
			delete_groupmem(fd, account);
			break;

		case '4':
			ban_groupuser(fd, account);
			break;

		case '5':
			unlock_ban(fd, account);
			break;

		case '6':
			srpthread_suspend();
			dissolve_group(fd, account);
			srpthread_resume();
			break;

		case '0':
			return 0;

		default:
			printf("cmd error!\n");
			sleep(1);
			break;
		}
	}
}

/*群功能函数 */
int group_func(int fd, char *name, char *account)
{
	char choice[SIZE];
	bzero(choice, sizeof(choice));

	while (1)
	{
		group_interface(name, account); //群界面
		scanf("%s", choice);
		switch (choice[0])
		{
		case '1': //显示我加入的群
			srpthread_suspend();
			show_groups(fd, account);
			srpthread_resume();
			break;

		case '2':
			send_groupmsg(fd, name);
			sleep(1);
			break;

		case '3': //建群
			srpthread_suspend();
			new_group(fd, name, account);
			srpthread_resume();
			break;

		case '4': //显示群成员
			srpthread_suspend();
			show_group_mem(fd, account);
			srpthread_resume();
			break;

		case '5': //加群
			srpthread_suspend();
			insert_group(fd, name, account);
			srpthread_resume();
			break;

		case '6': //退群
			exit_group(fd, name);
			break;

		case '7': //群管理
			group_manager(fd, name, account);
			break;

		case '0':
			return 0;

		default:
			printf("cmd error!\n");
			sleep(1);
			break;
		}
	}
}

/*接收消息 */
void *recv_message(void *arg)
{
	int ret;
	int fd = *((int *)arg);

	struct message *body = (struct message *)malloc(sizeof(struct message));
	while (1)
	{
		if (1 == pthread_death) //退出到登录界面后要杀死线程
		{
			pthread_death = 0;
			pthread_exit(NULL);
		}

		/*设置线程可挂起与继续 */
		pthread_mutex_lock(&mutex);
		while (flag <= 0)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);

		/*下方为函数内容 */

		bzero(body, sizeof(struct message));

		if ((ret = recv(fd, body, sizeof(struct message), 0)) < 0)
		{
			perror("recv");
			exit(1);
		}

		if (0 == ret)
		{
			printf("%d is close!\n", fd);
			pthread_exit(NULL);
		}

		if (EMPTYMSG == body->action)
		{
			continue;
		}

		if (SENDFILE == body->action)
		{
			struct message temp;
			memset(&temp, 0, sizeof(struct message));
			strcpy(temp.msg_a, body->msg_a);	//文件名
			strcpy(temp.msg_c, body->msg_c);	//发件人

			pthread_t id;
			if (pthread_create(&id, NULL, recvfile, (void *)&temp) != 0) //用于接收消息的线程
			{
				perror("pthread create error!");
				exit(1);
			}                                
			continue;
		}

		if (strcmp(body->msg_b, "not exist") == 0)
		{
			printf("no this user!\n");
			continue;
		}

		printf("user:%s\nsend message to you:\n%s\n", body->msg_c, body->msg_b);

		usleep(3);
	}

	pthread_exit(NULL);
}

/*处理函数 */
int main_handler(int fd)
{
	int mark = FALSE;
	char name[SIZE];
	bzero(name, sizeof(name));
	char account[SIZE];
	bzero(account, sizeof(account));

	while (1)
	{
		char choice[SIZE];
		bzero(choice, sizeof(choice));

		while (1)
		{
			Login(); //登录界面
			scanf("%s", choice);
			switch (choice[0])
			{
			case '1':
				mark = log_on(fd, name, account);
				break;

			case '2':
				reg_user(fd);
				break;

			case '3':
				mark = auto_logon(fd, name, account);
				break;

			case '4':
				find_pwd(fd);
				break;

			case '5':
				drop_user(fd);
				break;

			case '0':
				return 0;

			default:
				printf("cmd error!\n");
				sleep(1);
				break;
			}

			if (SUCCESS == mark) //验证正确进入用户界面
			{
				mark = FALSE;
				break;
			}
		}

		printf("login ...\n");

		pthread_t id;
		srpthread_init();
		if (pthread_create(&id, NULL, recv_message, (void *)(&fd)) != 0) //用于接收消息的线程
		{
			perror("pthread create error!");
			exit(1);
		}

		srpthread_resume(); //线程启动

		sleep(2); //睡眠以接收离线消息

		while (1)
		{
			char filename[SIZE];
			struct message *body = (struct message *)malloc(sizeof(struct message));
			bzero(body, sizeof(struct message));

			Using(name, account); //用户界面

			scanf("%s", choice);
			switch (choice[0])
			{
			case '1':
				srpthread_suspend(); //线程挂起
				showuser(fd);
				srpthread_resume();
				break;
			case '2':
				private_chat(fd, name);
				sleep(1);
				break;
			case '3':
				public_chat(fd, name);
				sleep(1);
				break;
			case '4': //群功能
				group_func(fd, name, account);
				break;
			case '5': //发文件
					  //send_file(fd, name);
				bzero(filename, sizeof(filename));

				body->action = SENDFILE;

				printf("input recver name:\n");
				scanf("%s", body->msg_b); //接收人
				printf("input file name:\n");
				scanf("%s", body->msg_a);  //传文件名
				strcpy(body->msg_c, name); //发送人

				send_fd(body, fd);

				strcpy(filename, body->msg_a);
#if 1
				if (pthread_create(&id, NULL, sendfile, (void *)filename) != 0) //用于接收消息的线程
				{
					perror("pthread create error!");
					exit(1);
				}
#endif
				break;
			case '8':
				srpthread_suspend();
				modify_pwd(fd, name); //修改密码
				srpthread_resume();
				sleep(2);
				mark = set_offline(fd, name); //修改密码后返回登录界面
				break;
			case '9':
				mark = set_offline(fd, name);
				break;
			case '0':
				return 0;
			default:
				printf("cmd error!\n");
				sleep(1);
				break;
			}

			if (SUCCESS == mark) //返回登录界面
			{
				mark = FALSE;
				pthread_death = 1; //杀死线程
				break;
			}
		}
	}
}
