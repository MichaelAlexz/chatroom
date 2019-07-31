#include <sqlite3.h>

#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#define SUCCESS 2000
#define FALSE 2001

struct online
{
    int cfd;
    char name[32];
    char account[32];

    struct online *next;
};

typedef struct online Node;
typedef struct online *Link;

struct tempfile
{
    int sockfd;
    char temp_a[64];
    char temp_b[64];
    char temp_c[64];
    Link head;
};

extern struct message *body;

void open_db(sqlite3 **);                       /*打开数据库 */
void InitDataBase(void);                        /*初始化数据库及创建表*/
void Insert_user(struct message *, int);        /*在表中插入注册人信息及账号*/
void Checkpwd(char *, int);                     /*搜查对应账号信息*/
void Findpwd(char *, int);                      /*通过账号名搜查对应账号信息*/
int InitTcp(void);                              /*tcp连接,绑定ip与端口*/
void Init_list(Link *);                         /*链表初始化 */
void insert_user(Link *, Link);                 /*在链表中插入登录人账号信息 */
int delete_user(Link *, int);                   /*删除用户结点 */
void show_user(int, Link);                      /*查找在线用户 */
int private_chat(struct message *, Link, int);  /*私聊 */
int public_chat(struct message *, Link);        /*向所有在线用户发消息 */
int check_account(struct message *, Link);      /*验证账户是否已在线 */
int reg_user(struct message *, int);            /*注册账号 */
void modify_pwd(struct message *, int);         /*修改密码 */
int exist_user(struct message *, int);          /*检查账户是否存在 */
void insert_offline_msg(struct message *);      /*插入离线消息 */
int send_offline_msg(struct message *, int);    /*验证与发送离线消息 */
void send_fd(struct message *, int);            /*发送 */
void recv_fd(struct message *, int);            /*接收 */
void GetId(char **);                            /*生成11位随机数*/
int new_group(struct message *, int);           /*建群 */
int drop_user(struct message *, int);           /*删除数据库中用户 */
int show_groups(struct message *, int);         /*显示已加入的群 */
int insert_groupmsg(struct message *, int);     /*加群 */
int exit_group(struct message *, int);          /*退群 */
int show_group_mem(struct message *, int);      /*显示所有群员 */
int send_groupmsg(struct message *, int, Link); /*发送群消息 */
int find_fd(char *, Link);                      /*根据用户名查找fd */
int show_mananger_group(struct message *, int); //显示我管理的群
int show_mangroup_mem(struct message *, int);   /*显示所有我管理的群成员 */
int delete_groupmem(struct message *, int);     //踢人
int ban_groupuser(struct message *, int);       //禁言
int unlock_ban(struct message *, int);          //解禁
int dissolve_group(struct message *, int);      //解散群
int recv_file(struct message *, int);           //收文件
int send_file(struct message *, Link);          //发文件
int InitTcp_file(void);                         /*tcp连接,绑定ip与端口,用于传输文件*/
int connect_fd(int);                            /*连接获取文件fd */
void *recvfile(void *arg);                      /*收文件线程 */
void *sendfile(void *arg);                      /*发文件线程 */

#endif
