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


int name_num=0;			//设置一个全局变量代表账号下标
int login=0;			// 设置一个全局变量代表接受消息类型是否为登录状态
int finally=0;				//设置一个全局变量代表下线的是否为尾结点
int flag_login=0;			//flag_login=1代表接受的是注册的消息
int delete=0;				//identity=代表管理员身份	
int flag_key=0;				//flag_key=1代表处于查看聊天记录状态
/*
 *函数声明部分
 *
 */
void my_err(const char *err_string,int line);		//错误处理
int find_name(const char *name, USERINFO users[]);	//查找用户名
int sign_in(THREAD *p, int flag_recv, USERINFO users[]);//登录函数
THREAD * find_match(THREAD *head, char *string);		//从在线列表中找出一个用户用于踢人和发消息
int message_pro(THREAD *thid,char *user ,char *info);	//判断一个消息是群聊还是私聊以及解析出纯消息
int write_list(THREAD * head);						//将在线列表写入文件
int userinfo_mat(char *f_username, char *s_username , int conn_fd);//查找聊天记录并将记录存储在records中然后发送至conn_fd
void wchat_records(char *filename, char *records);	//将聊天记录写入文件
void *thread1(THREAD *head);						//创建一个线程
int mychat_server(void);							//主要操控函数
int userinfo_match(char *re_username);				//注册部分用匹配用户名是否已经存在
int regis_account(char *re_username);				//找新用户名是否存在并回应相应信息()
int my_filewrite(USERINFO user_ss);					//将新用户名及密码写入文件
void create_file(void);								//保证操作过程中所需要的文件都存在
int read_user(USERINFO user[]);					    //从文件中读取出所有用户信息用于登录



//自定义的错误处理函数
void my_err(const char *err_string,int line)
{
	FILE *fp;

	fprintf(stderr, "line:%d ",line);
	perror(err_string);

	fp=fopen("error_log", "a+");
	if(fp==NULL){

		printf("error:92\n");
		exit(1);
	}

	fprintf(fp, "line:%d ",line);
	fprintf(fp, "%s:", err_string);
	fprintf(fp, "%d\n", __LINE__);

	exit(1);
}

//查找用户名是否存在，存在返回该用户名的下标，不存在则返回-1，出错返回-2
int find_name(const char *name, USERINFO users[])
{
	int i;
	int ret;
	if(name==NULL){
		printf("in find_name, NULL pointer\n");
		return -2;
	}
	for(i=0; users[i].username[0] != '\0'; i++ ){
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
int message_pro(THREAD *thid,char *user ,char *info)
{
	int i, j=0;
	int str_len=0;
	char *p;
	int key=0;

	str_len=strlen(thid->recv_buf);

	//找标志@
	for(i=0; i<str_len; i++){

		info[i]=thid->recv_buf[i];
		if( thid->recv_buf[i] == '@' ){
			key=1;
			break;
		}
	}
	info[i]='\0';

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

//查找聊天记录并将记录存储在records中然后发送至conn_fd
int userinfo_mat(char *f_username, char *s_username , int conn_fd)
{
	FILE *fp;			//文件指针
	int ret;			//ret=0表示没有聊天记录
	char ch='a';				//遍历文件
	char name1[128];			//从文件获得的发送方名
	char name2[128];			//从文件获得接收方用户名
	int i,j, k;
	char buf[128];		//存一条消息记录
	int x=0,y=0;		//逻辑值
	int key=0;			//key=0代表文件完全没有内容，key=1代表没有要查找的内容key=2代表找到
	int t=0;			//存储二维数组的下标
	char records[100][128];	//暂时存储聊天记录
	

	usleep(100);
	fp=fopen("ss_records", "rt");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}

	ch=fgetc(fp);

	if( ch != EOF){
		key=1;		//文件有内容
	}
	while( ch != EOF){

		i=0;
		j=0;
		k=0;
		//找出第一个名字
		while( (ch=fgetc(fp) ) !='@'){
			if( ch != '_' && ch !='\n' ){
				name1[i++]=ch;
			}
		}
		name1[i]='\0';

		//找出第二个名字
		while( ( ch=fgetc(fp) ) != ':'){
		name2[j++]=ch;
		}
		name2[j]='\0';

		//找出内容
		x=  ( !(strcmp(f_username, name1) ) &&  !( strcmp(s_username, name2) ) );
		y=  ( !(strcmp(f_username, name2) ) &&  !( strcmp(s_username, name1) ) );

		//printf("x=%d, y=%d\n", x, y);
		if( x || y  ){

			key=2;								//找到所需内容
			while(  (ch=fgetc(fp)) != '_'){
				buf[k++]=ch;
			}
			buf[k]='\0';

			//将聊天记录整理为所需要的格式
			strcat(records[t], name1);
			strcat(records[t], "@");
			strcat(records[t], name2);
			strcat(records[t], ":");
			strcat(records[t], buf);
		
			usleep(10);
			if( send(conn_fd, records[t], strlen(records[t])+1, 0) <0 ){
				my_err("send", __LINE__);
			}

		memset( records[t], 0, sizeof(records[t]) );
			t++;
		}

		//跳去多余不要的内容
		else{
			while( (ch=fgetc(fp))  != '_'){

			}
		}	

		//排除结尾符
		if( (ch=fgetc(fp)) == EOF){
			break;
		}
		if( (ch=fgetc(fp)) == EOF){
			break;
		}
	}


	if(key==0){
		if( send(conn_fd, "抱歉，暂时没有任何聊天记录...", 
					strlen("抱歉，暂时没有任何聊天记录...")+1, 0) <0 ){
			my_err("send", __LINE__);
		}
	}else if(key==1){
		if( send(conn_fd, "抱歉，暂时没有该用户和您的聊天记录，至少先说一句吧...", 
					strlen("抱歉，暂时没有该用户和您的聊天记录噢，至少先说一句吧...")+1, 0) <0 ){
			my_err("send", __LINE__);
		}
	}

	fclose(fp);
}


void wchat_records(char *filename, char *records)
{
	int fd;		//文件描述符

	//以追加的方式打开文件并写入
	if( (fd=open(filename, O_RDWR | O_APPEND) ) ==-1 ){
		my_err("fileopen", __LINE__);
	}

	if( write(fd, records, strlen(records)) != strlen(records) ){
		my_err("filewrite", __LINE__);
	}
	close(fd);
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
	char		records[100][128];	//用来储存聊天记录

	char        info[128];			//纯消息
	char		file_w[128];		//暂时保存需要向文件里面存的一条消息
	char		filename_s[32];		//私聊文件名
	char		filename_q[32];		//群聊文件名


	//设置文件名用于写入聊天记录
	strcpy(filename_s, "ss_records");
	strcpy(filename_q, "qq_records");

	thid=head->th;						//接受原本thid的值
	thid->stat=0;

	login=0;
	ret=read_user(users);					//从文件中读取用户信息用于登录
	while(1)
	{
		p=head->next;					//初始化指针变量

		thid->recv_buf[thid->len-1] = '\0';

		memset(thid->recv_buf, 0, sizeof(thid->recv_buf));
		thid->len=recv(thid->conn_fd, thid->recv_buf, sizeof(thid->recv_buf),0);
		//printf("%s\n", thid->recv_buf);
						
		//实施踢人
		if(thid->recv_buf[0] == '$' && thid->recv_buf[1] == '$'){
			thid->len=0;
		}

		//如果操作过程中用户下线
		if(thid->len == 0)
		{
			//从链表中删除该结点
			
			flag_key=0;
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

			write_list(head);
			pthread_exit(0);
		}

		else if(thid->recv_buf[0] == '\n'){
			continue;
		}
		//查看聊天记录
		else if(thid->recv_buf[0] == '#' && thid->recv_buf[1]  == '#'){
			flag_key=1;
			continue;
		}

		else if(flag_key == 1){
			
			printf("thid->pre_username=%s, thid->recv_buf=%s\n", thid->pre_username, thid->recv_buf);
			userinfo_mat(thid->pre_username, thid->recv_buf ,thid->conn_fd);
			flag_key=0;
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
	
		//接收的是踢人的提示
		else if(thid->recv_buf[0] == '^' && thid->recv_buf[1] == '^'){
			delete=1;
			continue;
		}
		else if(delete==1){

			check=find_match(head, thid->recv_buf);				//找到要踢的人
			if(check==NULL){
				if(send(thid->conn_fd, "Nonono\0", 7, 0) <0 ){
					my_err("send", __LINE__);
				}
				
				printf("no have such username\n");
			}
			else{
				if (send(check->conn_fd, "^^", strlen("^^")+1, 0) <0 ){
					my_err("scend", __LINE__);
				}
			}

			delete=0;
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
			continue;
		}
		
		//用achivement代表确认注册然后往文件里面写
		else if( strcmp(thid->recv_buf, "achivement") == 0){

			//返馈注册成功的信息给客户端
			if(send(thid->conn_fd, "Success\0", 8,0) <0 ){
				my_err("send", __LINE__);
			}

			//往文件写的函数
			my_filewrite(user_ss);
			
			strcpy(users[ret].username, user_ss.username);
			strcpy(users[ret].password, user_ss.password);

			ret++;
			flag_login=0;

		//	continue;
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

				flag_login=0;
			}
			else{
				//成功以后开始接受密码
				flag_login=2;
			}

			continue;
		}

		//接受的是密码
		else if(flag_login==2){
			strcpy(user_ss.password, thid->recv_buf);
			//假如密码内有东西就赋值，没有就重头开始
			flag_login=0;
			continue;
		}

		//如果接收到的是一条消息
		if( login == 1){
			//根据返回值key=1代表私发，key=0代表群发
			key=message_pro(head->present, string, info);
			if(key==1){
				check=find_match(head, string);				//匹配后的结点指针
				if(check==NULL){
					if(send(thid->conn_fd, "Nonono\0", 7, 0) <0 ){
						my_err("send", __LINE__);
					}
					printf("no have such username\n");
				}
				else{

					char pre_info[128];

					memset(pre_info, 0, strlen(pre_info));
					//将消息整合成指定格式，以便发送
					strcat(pre_info, thid->pre_username);
					strcat(pre_info, "@me: ");
					strcat(pre_info, info);

					//thid->recv_buf[strlen(thid->recv_buf)+1]='\0';
					//向指定的用户名发送消息
					if (send(check->conn_fd, pre_info, strlen(pre_info)+1, 0) <0 ){
						my_err("scend", __LINE__);
					}

					usleep(100);
					strcpy(file_w, "_\0");				//以_符号开头
					strcat(file_w, thid->pre_username);		//发消息的人

					strcat(file_w,"@\0");				
					strcat(file_w, string);				//连接接消息的人

					strcat(file_w, ":\0");				//消息分割处
					strcat(file_w, info);				//连接消息
					strcat(file_w, "_\n\0");			//结尾并加换行符

					wchat_records(filename_s, file_w);	//
					thid->recv_buf[0]='D';				//防止死循环
				}
				continue;

			}else if(key==0){
				//写入群文件
				strcat(file_w, thid->pre_username);
				strcat(file_w, ":\0");
				strcat(file_w, thid->recv_buf);
				strcat(file_w,"\n");
				wchat_records(filename_q, file_w);

				//将群消息整合成指定格式，以便发送
				char pre_info1[128];
				strcat(pre_info1, thid->pre_username);
				strcat(pre_info1, ":");
				strcat(pre_info1, thid->recv_buf);


				//群发
				while(p != NULL){
					//thid->recv_buf[strlen(thid->recv_buf)+1]='\0';
					if(p->conn_fd == thid->conn_fd){
						p=p->next;
						continue;
					}										//群消息自己不要重复收自己的
					else if (send(p->conn_fd, pre_info1, strlen(pre_info1)+1, 0) <0 ){
						my_err("scend", __LINE__);
					}
					p=p->next;
				}
			}
			
			
			//清零部分
			memset(file_w, 0, strlen(file_w));
			file_w[strlen(file_w)+1]='\0';
			file_w[0]='\0';


			fflush(stdin);
			//continue;
		}
	
		//接收到的是登录的用户名或者密码
		else if(login==0){
			i=sign_in(thid, flag_recv, users);
			if(i==1){
				strcpy(pre_username1,thid->recv_buf );
				flag_recv = PASSWORD;			//账户验证成功
			}
			else if(i==2){
				
				char pre_info2[128];		//储存用户上线提醒消息发送给在线用户
				//将密码和用户名赋值给链表的结点a
				strcpy(thid->pre_password, thid->recv_buf);
				strcpy(thid->pre_username,pre_username1);
				thid->stat=1;
				write_list(head);
				login=1;						//登录成功改变全局变量

				//整合成提示发送到每个用户
				strcpy(pre_info2, thid->pre_username);
				strcat(pre_info2, "上线");

				p=head->next;
				while(p != NULL){

					if(p->conn_fd == thid->conn_fd){
						p=p->next;
						continue;
					}										//群消息自己不要重复收自己的
					else if (send(p->conn_fd, pre_info2, strlen(pre_info2)+1, 0) <0 ){
						my_err("scend", __LINE__);
					}
					p=p->next;
				}

				memset( pre_info2, 0, sizeof(pre_info2) );
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

		//printf("accept a new client,ip:%s\n",inet_ntoa(cli_addr.sin_addr));
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

void create_file(void)
{

	int fd;
	//检查，文件不存在则新建
	fd=open("acc_pass", O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
	close(fd);


	fd=open("ss_records", O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
	close(fd);


	fd=open("qq_records", O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
	close(fd);
}
int main(void)
{

	printf("hah\n");
	create_file();
	mychat_server();

	return 0;
}
