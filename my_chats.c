/*************************************************************************
    > File Name: my_chats.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年08月07日 星期五 15时41分35秒
 ************************************************************************/
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/socket.h>
#include<string.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<arpa/inet.h>

#define FILEINFO 0		//操作的数据是文件
#define MESSAGE  1		//操作的数据是聊天消息

#define USERNAME 2		//接收到的是用户名	
#define PASSWORD 3		//接受到的是密码

#define SERV_PORT 4507  //服务器的端口
#define LISTMAX   12	//连接请求队列的最大长度

#define INVALID_USERINFO 'n'	//用户信息无效
#define VALID_USERINFO   'y'	//用户信息有效

typedef struct userinfo
{
	char username[32];
	char password[32];
}USERINFO;				//保存用户名和密码的结构体

//用于给新创建的线程传参的结构体
typedef struct th
{
	char recv_buf[1024]; //存消息的数组
	int len;			//获取文件的字节数
	pthread_t thread;		//线程号
	int conn_fd;		//一个已连接的套接字
	struct th *next;			//指向右边
	struct th *next1;			//指向左边
	char	pre_username[32];	//保存用户名
	char	pre_password[32];	//保存用户密码
	struct th *th;			//一个指针指向当前位置
	struct th *present;
	int stat;				//stat=1代表用户在线
}THREAD;		

typedef struct two_point
{
	THREAD *head;
	THREAD *th;
}POINT;

int name_num=0;			//设置一个全局变量代表账号下标
int login=0;			// 设置一个全局变量代表接受消息类型是否为登录状态
int finally=0;				//设置一个全局变量代表下线的是否为尾结点
int flag_login=0;			//flag_login=1代表接受的是注册的消息

/*
 *函数声明部分
 *
 */
void my_err(const char *err_string,int line);		//错误处理
int find_name(const char *name, USERINFO users[]);					//查找用户名
void *thread1(THREAD *head);						//创建一个线程
int mychat_server(void);							//主要操控函数
int message_pro(THREAD *thid,char *user);			//线程部分用户名处理
int userinfo_match(char *re_username);				//注册部分用户名匹配
int regis_account(char *re_username);				//找新用户名是否存在并回应相应信息
int my_filewrite(USERINFO user_ss);					//将新用户名及密码写入文件
int write_list(THREAD * head);						//将在线列表写入文件
int read_user(USERINFO user[]);					//从文件中读取用户信息用于登录


//自定义的错误处理函数
void my_err(const char *err_string,int line)
{
	fprintf(stderr, "line:%d ",line);
	perror(err_string);
	exit(1);
}

//查找用户名是否存在，存在返回该用户名的下标，不存在则返回-1，出错返回-2
int find_name(const char *name, USERINFO users[])
{
	int i;
	if(name==NULL){
		printf("in find_name, NULL pointer\n");
		return -2;
	}
	for(i=0; users[i].username[0] != ' '; i++ )
	{
		if( strcmp(users[i].username, name) ==0  ){
			return i;
		}
	}

	return -1;
}
void send_data(int conn_fd, const char *string)
{
	if( send(conn_fd, string, strlen(string), 0) <0 ){
		my_err("send",__LINE__);
		close(conn_fd);
		exit(1);
	}
}								//自定义发送数据函数

int sign_in(THREAD *p, int flag_recv, USERINFO users[])
{	
	//接受到的是用户名
	if(flag_recv==USERNAME){
		name_num=find_name(p->recv_buf, users);
		switch(name_num){
			case -1:
				send_data(p->conn_fd, "n\n");
				break;
			case -2:
				exit(-1);
				break;
			default:
				send_data(p->conn_fd, "y\n");
		        return 1;
		}

	}
	//接受到的是密码
	else if(flag_recv==PASSWORD){

		if(strcmp(users[name_num].password, p->recv_buf) ==0 ){
			send_data(p->conn_fd, "y\n");
			send_data(p->conn_fd, "welcome login my tcp server\n");
			printf("%s login\n", users[name_num].username);
					return 2;   //跳出while循环
		}else
			send_data(p->conn_fd,"n\n");
	}

	return 0;
}

//查找用户名所对应的结点从而找出套接字
THREAD * find_match(THREAD *head, char *string)
{
	THREAD *h;		
	h=head->next;

	while(h != NULL)
	{
		if (strcmp(h->pre_username,string) == 0 ){
			return h;
		}
		h=h->next;
	}

	return NULL;
}

//从接收到的消息中解析出用户名
int message_pro(THREAD *thid,char *user)
{
	int i, j=0;
	int str_len=0;
	char *p;
	int key=0;

	str_len=strlen(thid->recv_buf);

	//找标志@
	for(i=0; i<str_len; i++){
		if( thid->recv_buf[i] == '@' ){
			key=1;
			break;
		}
	}
	for(j=0; j<32; j++){
		i++;
		user[j]=thid->recv_buf[i];
		if(thid->recv_buf[i] == '\0')
			break;
	}
	user[++j]='\0';

	return key;
}

int write_list(THREAD * head)
{
	FILE *fp;
	THREAD *h=head->next;

	fp=fopen("list", "w");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}

	while( h != NULL){

		if(h->stat==1){
			fprintf(fp, "%s\n", h->pre_username);
		}
		h=h->next;
	}

	fclose(fp);
	return 0;
}
int  read_user(USERINFO user[])
{
	FILE *fp;
	int i=0;
	
	fp=fopen("acc_pass", "rt");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}
	while( fscanf(fp, "%s %s", user[i].username, user[i].password) != EOF){
		i++;
	}

	fclose(fp);

	return i;
}
void *thread1(THREAD *head)
{
	THREAD		*thid;
	THREAD		*check;
	THREAD		*p;				//指针用于遍历
	int			flag=-1;		//标志接受到的是文件还是消息
	int			i=0;
	int			key=-1;
	int flag_recv=USERNAME;
	char pre_username1[32];
	char        string[32];		//存储解析的用户名
	int			low=0;			//表示用户名和密码的下标
	USERINFO	user_ss;		//定义一个表示用户和密码的结构体变量
	USERINFO	users[100];		//从文件读取已注册用户
	int			ret=0;			//数组新注册用户的下标
	FILE		*fp;			//文件指针
	char		name[32];		//用来储存用户名

	thid=head->th;						//接受原本thid的值
	thid->stat=0;


	ret=read_user(users);					//从文件中读取用户信息用于登录
	while(1)
	{
		p=head->next;					//初始化指针变量

		memset(thid->recv_buf, 0, sizeof(thid->recv_buf));
		thid->len=recv(thid->conn_fd, thid->recv_buf, sizeof(thid->recv_buf), 0);

		//如果操作过程中用户下线
		if(thid->len == 0)
		{
			//从链表中删除该结点
			
			if(thid->next != NULL)
			{
				(thid->next1)->next=(thid->next);
				(thid->next)->next1=(thid->next1);
				free(thid);
			}

			else if(thid->next == NULL){
				(thid->next1)->next=NULL;
				free(thid);
				thid=NULL;
				finally=1;
			}

		//	pthread_detach( pthread_self() );
			//close(thid->conn_fd);		//先关闭再删除
			pthread_exit(0);
		}

		//查看聊天记录
		else if(thid->recv_buf[0] == '#' && thid->recv_buf[1]  == '#' || thid->recv_buf[0] == '\n'){
			continue;
		}

		//查看在线列表
		else if(thid->recv_buf[0] == '*' && thid->recv_buf[1]  == '*'){

			fp=fopen("list", "rt");
			if(fp==NULL){
				my_err("fopen", __LINE__);
			}

			while( fscanf(fp, "%s", name) != EOF){
				usleep(10);
				if( send(thid->conn_fd, name, strlen(name)+1, 0) <0 ){
					my_err("send", __LINE__);
				}
			}
			continue;
		}
		
		else if(thid->len<0){
			perror("recv");
			close(thid->conn_fd);		//记得找一下有close的地方
			exit(1);
		}

		thid->recv_buf[thid->len-1] = '\0';
		head->present=thid;			//保存当前的接收到的用户消息套接字	
		//printf("%s\n", thid->recv_buf);

	    if( strcmp(thid->recv_buf, "register") == 0){
			flag_login=1;
		}
		
		//用achivement代表确认注册然后往文件里面写
		else if( strcmp(thid->recv_buf, "achivement") == 0){
			my_filewrite(user_ss);
			
			strcpy(users[ret].username, user_ss.username);
			strcpy(users[ret].password, user_ss.password);

			ret++;
			//read_user(users);		//重新读取文件信息给数组
			flag_login=0;
		}

		//如果接收的是注册消息
		else if(flag_login==1){
			strcpy(user_ss.username, thid->recv_buf);
			//如果用户存在
			if(userinfo_match(thid->recv_buf) == 1){

				//赋值给结构体变量的用户名
				if(send(thid->conn_fd, "Username Already exists.Please choose another\0",47 ,0)  <0 ){
					my_err("send", __LINE__);
				}
			}
			else{
				//成功以后开始接受密码
				flag_login=2;
				if(send(thid->conn_fd, "Success\0", 8,0) <0 ){
					my_err("send", __LINE__);
				}
			}

		}

		//接受的是密码
		else if(flag_login==2){
			strcpy(user_ss.password, thid->recv_buf);
			//假如密码内有东西就赋值，没有就重头开始
			flag_login=0;
		}

		//如果接收到的是一条消息
		else if( login == 1){
			//根据返回值key=1代表私发，key=0代表群发
			key=message_pro(head->present, string);
			if(key==1){
				check=find_match(head, string);				//匹配后的结点指针
				if(check==NULL){
					if(send(thid->conn_fd, "Nonono\0", 7, 0) <0 ){
						my_err("send", __LINE__);
					}
					printf("no have such username\n");
				}
				else{
					thid->recv_buf[strlen(thid->recv_buf)+1]='\0';
					//向指定的用户名发送消息
					if (send(check->conn_fd, thid->recv_buf, strlen(thid->recv_buf)+1, 0) <0 ){
						my_err("scend", __LINE__);
					}
					//	printf("%d\n", check->conn_fd);		//测试找结点有没有找正确
						thid->recv_buf[0]='D';				//防止死循环
				}
				continue;

			}else if(key==0){
				while(p != NULL){

					thid->recv_buf[strlen(thid->recv_buf)+1]='\0';
					if(p->conn_fd == thid->conn_fd){
						p=p->next;
						continue;
					}										//群消息自己不要重复收自己的
					else if (send(p->conn_fd, thid->recv_buf, strlen(thid->recv_buf)+1, 0) <0 ){
						my_err("scend", __LINE__);
					}
				//	printf("%s\n", p->pre_username);
					p=p->next;
				}
			}

		}

		//接收到的是用户名或者密码
		else if(login==0){
			i=sign_in(thid, flag_recv, users);
			if(i==1){
				strcpy(pre_username1,thid->recv_buf );
				flag_recv = PASSWORD;			//账户验证成功
			}
			else if(i==2){

				//将密码和用户名赋值给链表的结点a
				strcpy(thid->pre_password, thid->recv_buf);
				strcpy(thid->pre_username,pre_username1);
				thid->stat=1;
				write_list(head);
				login=1;						//登录成功改变全局变量
			}
		}
	}

	pthread_exit(0);
}

int mychat_server(void)
{
	int sock_fd;
	int optval;		     	//设置套接字时用到
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	socklen_t	clilen;
	int ret;
	POINT pt;


	THREAD *thid;			//定义一个指向结构体的指针
	THREAD *tail;
	THREAD *head;
	head=(THREAD *)malloc(sizeof(THREAD));
	head->len=0;		
	THREAD *p;
	//head->next=NULL;
	head->next1=NULL;

	tail=head;				//用于创建一条带头结点的链表

	//创建一个TCP套接字
	sock_fd=socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd==0){
		my_err("socket", __LINE__);
	}

	//设置套接字使之可以重新绑定端口,第二个参数代表通用套接字
	optval=1;
	if( setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval,sizeof(int))<0){
		my_err("setsockopt", __LINE__);
	}
    //初始化服务器端地址结构
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(SERV_PORT);
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	//将套接字绑定到本地端口
	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) <0 ){
		my_err("bind", __LINE__);
	}

	//将套接字转化为监听套接字
	if(listen(sock_fd, LISTMAX) <0 ){
		my_err("listen", __LINE__);
	}

	clilen=sizeof(struct sockaddr_in);

	while(1){
		
	   thid=(THREAD *)malloc(sizeof(THREAD));	//动态申请一段内存空间
	   if(thid==NULL){
		   printf("failed to apply for memory space\n");
		   exit(1);
	   }
		thid->conn_fd=accept(sock_fd, (struct sockaddr*)&cli_addr, &clilen );
		if(thid->conn_fd <0 ){
			my_err("accept",__LINE__);
		}										//得到代表客户端的套接字

		login=0;								//初始化全局变量保证每次先登录
		printf("%d\n", thid->conn_fd);

		//给链表建立双向关系	
		p=head->next;
		/*printf("\n打印链表\n");
		while(p != NULL)
		{
			printf("%s\n", p->pre_username);
			p=p->next;
		}
*/
		if(finally==1 ){

			printf("1234\n");
			tail=tail->next1;
			finally=0;
		}
		thid->next1=tail;
		thid->next=NULL;

		tail->next=thid;
		tail=thid;

		head->th=thid;

		printf("accept a new client,ip:%s\n",inet_ntoa(cli_addr.sin_addr));
		if( pthread_create(&(thid->thread),  NULL, (void *)thread1, head)  != 0){
				printf("thread create failed\n");
				exit(1);
		}
	}
}

//读文件的内容与需要注册的用户名匹配
int userinfo_match(char *re_username)
{
	FILE *fp;							//文件指针
	int i=0;							//i用于遍历
	USERINFO	user[100];				//定义一个结构体数组保存用户名和密码

	fp=fopen("acc_pass", "rt");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}

	while( fscanf(fp, "%s %s", user[i].username, user[i].password) != EOF)
	{
		if( strcmp(user[i].username, re_username) ==0 ){

			fclose(fp);
			return 1;					//1代表用户名已经存在了
		}
		i++;
	}
	fclose(fp);
	return 0;									//代表用户名不存在可以注册
}

//将新用户名写入文件
int regis_account(char *re_username)	//re_username代表准备注册的用户名
{
	int ret=0;							//ret=1代表用户名存在

	ret=userinfo_match(re_username);
	//如果用户名不存在则将用户名写入文件
	if(ret==1){
		printf("already have \n");
	}
	return ret;
}

//将用户名及密码写入文件
int my_filewrite(USERINFO user_ss)
{
	FILE *fp;

	fp=fopen("acc_pass", "a+");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}

	fprintf(fp, "%s %s\n", user_ss.username, user_ss.password);
	fclose(fp);
}

/*void my_read()
{
	FILE *fp;
	char ch;
	fp=fopen("acc_pass", "a+");
	if(fp == NULL){
		printf("打开文件失败\n");
		exit(1);
	}

	while( (ch=fgetc(fp)) != EOF){
		printf("%c", ch);
	}

}*/
int main(void)
{

//	my_read();
	/*char string[32];
	scanf("%s", string);
	regis_account(string);


// 	regis_account();
*/
	printf("hah\n");
	mychat_server();

//	 my_read();
	return 0;
}
