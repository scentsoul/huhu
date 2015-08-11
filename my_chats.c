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
	struct th *next;
	struct th *next1;
	char	pre_username[32];	//保存用户名
	struct th *th;			//一个指针指向当前位置
	struct th *present;
}THREAD;		

typedef struct two_point
{
	THREAD *head;
	THREAD *th;
}POINT;
int name_num=0;			//设置一个全局变量代表账号下标
struct userinfo users[]={
	{
		"linux","unix"
	},
	{
		"4057","4058"
	},
	{
		"clh","clh"
	},
	{
		"xl","xl"
	},
	{
		" "," "//以一个只含空格的字符串作为数组结束的标志
	}
};

/*
 *函数声明部分
 *
 */

void my_err(const char *err_string,int line);		//错误处理
int find_name(const char *name);					//查找用户名
void *thread1(THREAD *head);						//创建一个线程
int mychat_server(void);							//主要操控函数
int message_pro(THREAD *thid,char *user);			//用户名处理

//自定义的错误处理函数
void my_err(const char *err_string,int line)
{
	fprintf(stderr, "line:%d ",line);
	perror(err_string);
	exit(1);
}

//查找用户名是否存在，存在返回该用户名的下标，不存在则返回-1，出错返回-2
int find_name(const char *name)
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
	}
}								//自定义发送数据函数

int sign_in(THREAD *p, int flag_recv)
{	
	//接受到的是用户名
	if(flag_recv==USERNAME){
		name_num=find_name(p->recv_buf);
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
THREAD * find_match(THREAD *head)
{
	THREAD *h;		
	char string[32];
	h=head->next;

	message_pro(head->present, string);  //有改动
	//puts(string);                        //测试函数调用有没有成功
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


	//printf("recv_buf = %s\n", thid->recv_buf);
	str_len=strlen(thid->recv_buf);

	for(i=0; i<str_len; i++){
		if( thid->recv_buf[i] == '@' ){
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

	//printf("%s\n", user);			//测试用户名是否解析正确
	
	return 0;
}
void *thread1(THREAD *head)
{
	THREAD		*thid;
	THREAD		*check;
	int			flag=-1;		//标志接受到的是文件还是消息
	int			i=0;
	int flag_recv=USERNAME;
	char pre_username1[32];

	thid=head->th;						//接受原本thid的值
	
	while(1)
	{
		memset(thid->recv_buf, 0, sizeof(thid->recv_buf));
		thid->len=recv(thid->conn_fd, thid->recv_buf, sizeof(thid->recv_buf), 0);

		//如果操作过程中用户下线
		if(thid->len == 0)
		{

			close(thid->conn_fd);		//先关闭再删除
			//从链表中删除该结点
			
			if(thid->next != NULL)
			{
				(thid->next1)->next=(thid->next);
				(thid->next)->next1=(thid->next1);
				free(thid);
			}

			else if(thid->next == NULL){
				(thid->next1)->next=NULL;
				thid->next1=NULL;
				free(thid);
			}
			pthread_exit(0);
		}
		else if(thid->len<0){
			perror("recv");
			exit(1);
		}
		thid->recv_buf[thid->len-1] = '\0';
		head->present=thid;			//保存当前的接收到的用户消息套接字	
		//printf("%s\n", thid->recv_buf);
		
		//如果接收到的是一个文件
	    if(thid->recv_buf[0] == 'F'){
		}

		//如果接收到的是一条消息
		else if(thid->recv_buf[0] == 'M' ){

			check=find_match(head);				//匹配后的结点指针
			if(check==NULL){
				printf("no have such username\n");
			}
			else{
				thid->recv_buf[strlen(thid->recv_buf)+1]='\0';
				printf("%s12345\n", thid->recv_buf);
				if (send(check->conn_fd, thid->recv_buf, strlen(thid->recv_buf)+1, 0) <0 ){
					my_err("scend", __LINE__);
					//exit(0);
				}
			//	printf("%d\n", check->conn_fd);		//测试找结点有没有找正确
			//
				thid->recv_buf[0]='D';				//防止死循环
			//	exit(0);
			}
		}

		//接收到的是用户名或者密码
		else{
			i=sign_in(thid, flag_recv);
			if(i==1){
				strcpy(pre_username1,thid->recv_buf );
				flag_recv = PASSWORD;			//账户验证成功
			}
			else if(i==2){
				strcpy(thid->pre_username,pre_username1);
			//	break;							//登录成功跳出循环
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
	head->next=NULL;
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

		printf("%d\n", thid->conn_fd);

		//给链表建立双向关系
		thid->next1=tail;
		thid->next=NULL;

		tail->next=thid;
		tail=thid;

		head->th=thid;

		printf("accept a new clien,ip:%s\n",inet_ntoa(cli_addr.sin_addr));
		if( pthread_create(&(thid->thread),  NULL, (void *)thread1, head)  != 0){
				printf("thread create failed\n");
				exit(1);
		}
	}
}

int main(void)
{
	printf("hah\n");
    mychat_server();
	
	return 0;
}
