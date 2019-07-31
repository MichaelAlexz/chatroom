#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../header.h"
#include "client_header.h"

/*用于发送文件线程 */
void *sendfile(void *arg)
{
	char *filename = (char *)arg;

	printf("connecting...\n"); //等待服务器端准备ok
	sleep(2);

	int fd_send = InitTcp_file();

	printf("connect to %d success...\n", fd_send);

	int count = 0; //读取次数
	int ret;

	struct filemsg file;
	bzero(&file, sizeof(file));

	FILE *fd_r = fopen(filename, "r"); //创建文件描述符

	printf("send file start...\n");

	while ((file.len = fread(file.msg, 1, sizeof(file.msg), fd_r)) > 0)
	{
		if ((ret = send(fd_send, &file, sizeof(file), 0)) < 0)
		{
			perror("send");
			exit(1);
		}

		usleep(700);

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

	printf("send file success...\n");
	printf("count: %d\n", count);
	sleep(1);

	pthread_exit(NULL);
}

/*收文件线程 */
void *recvfile(void *arg)
{
	struct message *body = (struct message *)arg;

	printf("you have a file: %s to recv...\n", body->msg_a);
	printf("connecting...\n"); //等待服务器端准备ok
	sleep(2);

	int fd_recv = InitTcp_file();

	struct filemsg file;
	int ret;
	int count = 0; //读取次数

	FILE *fd_w = fopen(body->msg_a, "w+");

	printf("recv %s start...\n", body->msg_a);

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
	close(fd_recv);//////

	printf("count :%d\n", count);

	printf("recv %s end...\n", body->msg_a);

	pthread_exit(NULL);
}

/*收文件 */
int recv_file(struct message *body, int fd)
{
	int ret;
	struct filemsg file;
	int count = 0;

	printf("user:%s\tsend file to you\n", body->msg_c);
	FILE *fd_w = fopen(body->msg_a, "w+");

	while (recv(fd, &file, sizeof(file), 0) > 0)
	{
		if (strcmp(file.msg, "end") == 0)
			break;

		ret = fwrite(file.msg, 1, file.len, fd_w);
		if (ret < 0)
		{
			printf("fwrite error!\n");
			return FALSE;
		}

		count++;

		//printf("goal: %d\t result: %d\t count: %d\n",file.len, ret, count);
		bzero(&file, sizeof(file));
	}

	fclose(fd_w);

	printf("count:%d\n", count);
	printf("recv %s success...\n", body->msg_a);
	sleep(1);

	return SUCCESS;
}
