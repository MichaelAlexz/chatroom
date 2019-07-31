#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "../header.h"
#include "server_header.h"

extern time_t timep;
extern int sockfd_file;

/*收文件线程 */
void *recvfile(void *arg)
{
	struct tempfile *body = (struct tempfile *)arg;

	int fd_recv = connect_fd(sockfd_file); //建立新通道

	printf("connect to %d success...\n", fd_recv);

	struct filemsg file;
	int ret;
	int count = 0; //读取次数

	FILE *fd_w = fopen(body->temp_a, "w+");

	time(&timep);
	printf("recv %s start...\t%s\n", body->temp_a, ctime(&timep));

	while (recv(fd_recv, &file, sizeof(file), 0) > 0)
	{
		if (strcmp(file.msg, "end") == 0)
			break;

		ret = fwrite(file.msg, 1, file.len, fd_w);

		if (ret < 0)
		{
			printf("fwrite error!\n");
			pthread_exit(NULL);
		}

		count++;

		//printf("goal: %d\t result: %d\t count: %d\n",file.len, ret, count);
		bzero(&file, sizeof(file));
	}

	fclose(fd_w);
	close(fd_recv);

	printf("count :%d\n", count);
	time(&timep);
	printf("recv %s end...\t%s\n", body->temp_a, ctime(&timep));

	pthread_t id;
	if (pthread_create(&id, NULL, sendfile, (void *)body) != 0)
	{
		perror("pthread create error!");
		time(&timep);
		printf("%s\n", ctime(&timep));
		exit(1);
	}

	pthread_exit(NULL);
}

/*发文件线程 */
void *sendfile(void *arg)
{
	struct tempfile *temp = (struct tempfile *)arg;
	struct message body;

	int fd_recver = find_fd(temp->temp_b, temp->head);

	memset(&body, 0, sizeof(body));

	body.action = SENDFILE;
	strcpy(body.msg_a, temp->temp_a); //文件名
	strcpy(body.msg_c, temp->temp_c); //发送人

	send_fd(&body, fd_recver); //发送基本信息让客户端准备

	int fd_send = connect_fd(sockfd_file); //建立新通道

	printf("connect to %d success...\n", fd_send);

	struct filemsg file;
	int ret;
	int count = 0; //读取次数

	FILE *fd_r = fopen(body.msg_a, "r");

	printf("send file start...\n");

	while ((file.len = fread(file.msg, 1, sizeof(file.msg), fd_r)) > 0)
	{
		if ((ret = send(fd_send, &file, sizeof(file), 0)) < 0)
		{
			perror("send");
			exit(1);
		}

		usleep(500);

		count++;
		//printf("filelen: %d\t count:%d\n", file.len, count);
		bzero(&file, sizeof(file));
	}

	strcpy(file.msg, "end");
	if ((ret = send(fd_send, &file, sizeof(file), 0)) < 0)
	{
		perror("send");
		exit(1);
	}

	fclose(fd_r);
	close(fd_send); 

	printf("send file success...\n");
	printf("count: %d\n", count);
	sleep(1);

	pthread_exit(NULL);
}

/*收文件 */
int recv_file(struct message *body, int cfd)
{
	printf("recv success...\n");

	return SUCCESS;
}

/*发文件*/
int send_file(struct message *body, Link head)
{
	int ret;
	int count = 0; //读取次数

	struct filemsg file;
	bzero(&file, sizeof(file));

	body->action = SENDFILE;

	int cfd = find_fd(body->msg_b, head);

	send_fd(body, cfd);

	FILE *fd_r = fopen(body->msg_a, "r"); //创建文件描述符

	while ((file.len = fread(file.msg, 1, sizeof(file.msg), fd_r)) > 0)
	{
		if ((ret = send(cfd, &file, sizeof(file), 0)) < 0)
		{
			perror("send");
			exit(1);
		}

		usleep(500);

		count++;
		//printf("filelen: %d\t count:%d\n", file.len, count);
		bzero(&file, sizeof(file));
	}

	strcpy(file.msg, "end");
	if ((ret = send(cfd, &file, sizeof(file), 0)) < 0)
	{
		perror("send");
		exit(1);
	}

	fclose(fd_r);

	printf("count: %d\n", count);
	time(&timep);
	printf("send %s success...\t%s\n", body->msg_a, ctime(&timep));

	free(body);
	return SUCCESS;
}