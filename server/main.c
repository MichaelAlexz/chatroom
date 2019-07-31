#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "../header.h"
#include "server_header.h"

#define PORT 33333
#define ID_LEN 11

int sockfd_file;

time_t timep;

Link head;

/*发送 */
void send_fd(struct message *body, int cfd)
{
	if (send(cfd, body, sizeof(struct message), 0) < 0)
	{
		perror("send");
		time(&timep);
		printf("%s\n", ctime(&timep)); //显示时间
		exit(1);
	}
}

/*接收 */
void recv_fd(struct message *body, int cfd)
{
	if (recv(cfd, body, sizeof(struct message), 0) < 0)
	{
		perror("recv");
		time(&timep);
		printf("%s\n", ctime(&timep)); //显示时间
		exit(1);
	}
}
/*
生成11位随机数
*/
void GetId(char **id)
{
	char str[ID_LEN + 1];
	bzero(str, sizeof(str));

	int i;
	int flag;
	srand(time(NULL));

	for (i = 0; i < ID_LEN; i++)
	{
		flag = rand() % 3;
		switch (flag)
		{
		case 0:
			str[i] = rand() % 10 + '0';
			break;
		case 1:
			str[i] = rand() % 10 + '0';
			break;
		case 2:
			str[i] = rand() % 10 + '0';
			break;
		}
	}

	strcpy(*id, str);

	time(&timep);
	printf("getid success\t%s\n", ctime(&timep));
}

/*action筛选 */
void Action_switch(int cfd, struct message *body)
{
	Link new = (Link)malloc(sizeof(Node));

	switch (body->action)
	{
	case REG:
		reg_user(body, cfd);

		break;

	case LOGIN:
		if (exist_user(body, cfd) == FALSE)
			break;

		if (check_account(body, head) == FALSE)
		{
			body->action = ONLINE;
			send_fd(body, cfd);
			break;
		}

		Checkpwd(body->msg_a, cfd);
		break;

	case LOGIN_SUCCESS: //验证成功后插入链表显示在线状态
		bzero(new, sizeof(Node));

		new->cfd = cfd;
		strcpy(new->name, body->msg_a);
		strcpy(new->account, body->msg_c);

		insert_user(&head, new);

		send_offline_msg(body, cfd); //验证与发送离线消息
		break;

	case AUTOLOG:
		if (exist_user(body, cfd) == FALSE)
			break;

		if (check_account(body, head) == FALSE)
		{
			body->action = ONLINE;
			send_fd(body, cfd);
			break;
		}

		Checkpwd(body->msg_a, cfd);
		break;

	case FINDLOST:
		Findpwd(body->msg_a, cfd);
		break;

	case DROPUSER:
		drop_user(body, cfd);
		break;

	case SHOWMEMBER:
		show_user(cfd, head);
		break;

	case PRIVATECHAT:
		private_chat(body, head, cfd);
		break;

	case PUBLICCHAT:
		public_chat(body, head);
		break;

	case MODIFYPWD:
		modify_pwd(body, cfd);
		break;

	case OFFLINE:
		body->action = EMPTYMSG;
		send_fd(body, cfd); //发送一个空结构指针来解决客户端阻塞接收的问题

		delete_user(&head, cfd);
		break;

	case NEWGROUP: //建群
		new_group(body, cfd);
		break;

	case SHOWGROUPS: //显示加入的群
		show_groups(body, cfd);
		break;

	case JOINGROUP: //加群
		insert_groupmsg(body, cfd);
		break;

	case EXITGROUP: //退群
		exit_group(body, cfd);
		break;

	case SHOWGROUPMEM: //显示所有群员
		show_group_mem(body, cfd);
		break;

	case SENDGROUPMSG: //发送群消息
		send_groupmsg(body, cfd, head);
		break;

	case SHOWMANAGERGROUP: //显示我管理的群
		show_mananger_group(body, cfd);
		break;

	case SHOWMANGROUPMEM: //显示我管理的群成员
		show_mangroup_mem(body, cfd);
		break;

	case DELETEGROUPMEM:
		delete_groupmem(body, cfd); //踢人
		break;

	case BANGROUPUSER: //禁言
		ban_groupuser(body, cfd);
		break;

	case UNLOCKBAN:
		unlock_ban(body, cfd);
		break;

	case DISSOLVEGROUP: //解散群
		dissolve_group(body, cfd);
		break;
#if 0
	case SENDFILE:
		recv_file(body, cfd); //收文件
		//send_file(body, head); //发文件
		break;
#endif
	}
}

/*
线程，处理以及收发消息，数据库修改
*/
void *handle_message(void *arg)
{
	int ret;
	pthread_t id; ///////////////////
	int cfd = *((int *)arg);

	struct message *body = (struct message *)malloc(sizeof(struct message));

	while (1)
	{
		bzero(body, sizeof(struct message));

		if ((ret = recv(cfd, body, sizeof(struct message), 0)) < 0)
		{
			perror("recv");
			time(&timep);
			printf("%s\n", ctime(&timep)); //显示时间
			exit(1);
		}

		if (ret == 0)
		{
			time(&timep);
			printf("%d is close!\t%s\n", cfd, ctime(&timep));

			delete_user(&head, cfd);

			pthread_exit(NULL);
		}

		Action_switch(cfd, body);
#if 1
		if (body->action == SENDFILE) //收发文件线程因为生命周期问题写在循环中
		{
			struct tempfile temp;
			strcpy(temp.temp_a, body->msg_a);
			strcpy(temp.temp_b, body->msg_b);
			strcpy(temp.temp_c, body->msg_c);
			temp.head = head;

			if (pthread_create(&id, NULL, recvfile, (void *)&temp) != 0)
			{
				perror("pthread create error!");
				time(&timep);
				printf("%s\n", ctime(&timep));
				exit(1);
			}
		}
#endif

		usleep(3);
	}

	pthread_join(id, NULL);

	pthread_exit(NULL);
}

/*接收与处理消息 */
void handle_main(int sockfd)
{
	int cfd; //通讯符
	int c_len;
	struct sockaddr_in c_addr; //用于打印连接方信息
	char buffer[1024];

	pthread_t id;

	while (1)
	{
		bzero(buffer, sizeof(buffer));

		bzero(&c_addr, sizeof(struct sockaddr_in));
		c_len = sizeof(struct sockaddr_in);

		time(&timep);
		printf("accepting...\t%s\n", ctime(&timep));

		if ((cfd = accept(sockfd, (struct sockaddr *)(&c_addr), &c_len)) < 0)
		{
			perror("accept error");
			time(&timep);
			printf("%s\n", ctime(&timep));
			exit(1);
		}

		printf("port = %d ip = %s\n", ntohs(c_addr.sin_port), inet_ntoa(c_addr.sin_addr));

		if (pthread_create(&id, NULL, handle_message, (void *)(&cfd)) != 0)
		{
			perror("pthread create error!");
			time(&timep);
			printf("%s\n", ctime(&timep));
			exit(1);
		}

		usleep(3);
	}
}

int main()
{
	/* 
	int fd_w = open("server.log", O_WRONLY | O_APPEND);
	if (fd_w < 0)
	{
		perror("open");
		exit(1);
	}

	dup2(1, fd_w); //重定向*/

	InitDataBase(); //初始化数据库

	Init_list(&head); //初始化链表

	int sockfd = InitTcp();

	sockfd_file = InitTcp_file();	//生成传文件用sockfd

	handle_main(sockfd); //处理函数

	return 0;
}
