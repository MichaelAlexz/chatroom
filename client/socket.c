#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 5555
#define PROT_F 8888

#define ADDRESS "192.168.1.217"

/*
tcp连接
绑定ip与端口
*/
int InitTcp(void)
{
	int sockfd;
	
	struct sockaddr_in addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}
    
	printf("client socket success...\n");

    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ADDRESS);

    if(connect(sockfd,(struct sockaddr *)(&addr),sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect error");
		exit(1);
    }

    printf("connect success...\n");

	return sockfd;
}

/*
tcp连接
绑定ip与端口,用于传输文件用
*/
int InitTcp_file(void)
{
	int sockfd;
	
	struct sockaddr_in addr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}
    
	printf("client socket success...\n");

    bzero(&addr,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PROT_F);
    addr.sin_addr.s_addr = inet_addr(ADDRESS);

    if(connect(sockfd,(struct sockaddr *)(&addr),sizeof(struct sockaddr_in)) < 0)
    {
        perror("connect error");
		exit(1);
    }

    printf("connect success...\n");

	return sockfd;
}
