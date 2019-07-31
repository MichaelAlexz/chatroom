#ifndef HEADER_H
#define HEADER_H
/*
服务器与客户端都用到的结构体以及宏
*/

//登录界面用
#define REG 1000
#define LOGIN 1001
#define AUTOLOG 1002
#define FINDLOST 1004
#define LOGIN_SUCCESS 1005
#define ONLINE 1009
#define DROPUSER 1013

//用户界面用
#define SHOWMEMBER 1003
#define PRIVATECHAT 1006
#define EMPTYMSG 1007
#define PUBLICCHAT 1008
#define MODIFYPWD 1010
#define OFFLINE 1011
#define NEWGROUP 1012
#define SHOWGROUPS 1014
#define JOINGROUP 1015
#define EXITGROUP 1016
#define SHOWGROUPMEM 1017
#define SENDGROUPMSG 1018
#define SHOWMANAGERGROUP 1019
#define SHOWMANGROUPMEM 1020
#define DELETEGROUPMEM 1021
#define BANGROUPUSER 1022
#define UNLOCKBAN 1023
#define DISSOLVEGROUP 1024
#define SENDFILE 1025

#define SIZE 128

struct message
{
	int action;
	char msg_a[SIZE];
	char msg_b[SIZE];
	char msg_c[SIZE];
	char msg_d[SIZE];
};

struct filemsg
{
	int len;
	char msg[1024];
};

#endif
