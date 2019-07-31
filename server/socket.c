#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5555
#define PORT_F 8888
#define ADDRESS "192.168.1.217"

extern time_t timep;

/*
tcp连接
绑定ip与端口
*/
int InitTcp(void)
{
    int sockfd;

    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    time(&timep);
    printf("socket success...\t%s\n", ctime(&timep));

    int opt = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(ADDRESS);

    if (bind(sockfd, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    time(&timep);
    printf("bind success...\t%s\n", ctime(&timep));

    if (listen(sockfd, 3) < 0)
    {
        perror("listen error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    time(&timep);
    printf("listen success...\t%s\n", ctime(&timep));

    return sockfd;
}

/*
tcp连接
绑定ip与端口,用于传输文件
*/
int InitTcp_file(void)
{
    int sockfd;

    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("f socket error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    time(&timep);
    printf("f socket success...\t%s\n", ctime(&timep));

    int opt = 1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_F);
    addr.sin_addr.s_addr = inet_addr(ADDRESS);

    if (bind(sockfd, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in)) < 0)
    {
        perror("f bind error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }


    time(&timep);
    printf("f bind success...\t%s\n", ctime(&timep));

    if (listen(sockfd, 3) < 0)
    {
        perror("f listen error!");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    time(&timep);
    printf("f listen success...\t%s\n", ctime(&timep));

    return sockfd;
}

/*连接获取文件fd */
int connect_fd(int sockfd)
{
    int c_len;
    int fd_file;

    struct sockaddr_in c_addr; //用于打印连接方信息
    bzero(&c_addr, sizeof(struct sockaddr_in));

    if ((fd_file = accept(sockfd, (struct sockaddr *)(&c_addr), &c_len)) < 0)
    {
        perror("f accept error");
        time(&timep);
        printf("%s\n", ctime(&timep));
        exit(1);
    }

    //printf("port = %d ip = %s\n", ntohs(c_addr.sin_port), inet_ntoa(c_addr.sin_addr));
    printf("fd_file get success...\n");

    return fd_file;
}