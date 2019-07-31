#include <sqlite3.h>

#ifndef CLIENT_HEADER_H
#define CLIENT_HEADER_H

#define SUCCESS 2000
#define FALSE 2001
#define MODIFY 2002

extern struct message *body;

void open_db(sqlite3 **);              /*打开数据库 */
int main_handler(int);                 /*处理函数 */
void InitDataBase(void);               /*初始化数据库*/
void Insert_user(struct message *);    /*记住密码，将账号以及密码记入数据库*/
int Search_user(struct message *);     /*搜索用户是否记录过*/
void Search_pwd(struct message *);     /*搜索用户密码*/
int InitTcp(void);                     /*tcp连接,绑定ip与端口*/
int checkpwd(char *);                  /*检查密码*/
int log_on(int, char *, char *);       /*验证登录用 */
int reg_user(int);                     /*注册用户 */
int auto_logon(int, char *, char *);   /*自动登录 */
void find_pwd(int);                    /*找回密码 */
void showuser(int);                    /*显示在线用户 */
void private_chat(int, char *);        /*私聊 */
void public_chat(int, char *);         /*给所有在线人发送消息 */
void modify_pwd(int, char *);          /*修改密码 */
void delete_user(struct message *);    /*删除用户 */
int set_offline(int, char *);          /*返回登录界面 */
void send_fd(struct message *, int);   /*发送 */
void recv_fd(struct message *, int);   /*接收 */
int new_group(int, char *, char *);    /*建群 */
int drop_user(int);                    /*注销用户 */
int show_groups(int, char *);          /*显示我的群 */
int insert_group(int, char *, char *); /*加群 */
int exit_group(int, char *);           /*退群 */
int show_group_mem(int, char *);       /*显示群成员 */
int send_groupmsg(int, char *);        /*发送群消息 */
void getTime(char *);                  /*获得时间 */
int show_mananger_group(int, char *);  /*显示我管理的群 */
int show_mangroup_mem(int, char *);    /*显示我管理的群成员 */
int delete_groupmem(int, char *);      /*踢人 */
int ban_groupuser(int, char *);        /*禁言 */
int unlock_ban(int, char *);           /*解开禁言 */
int dissolve_group(int, char *);       /*解散群 */
int recv_file(struct message *, int);  /*收文件 */
int InitTcp_file(void);                /*tcp连接,绑定ip与端口,用于传输文件用*/
void *sendfile(void *arg);             /*用于发送文件线程 */
void *recvfile(void *arg);             /*收文件线程 */

#endif
