/*************************************************************************
    > File Name: my_client.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年08月06日 星期四 14时47分05秒
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
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/stat.h>
#include"my_recv.h"

int flag_key=1;			//flag_key=1代表消息发送对象存在
int flag_login=0;		//flag_login=0代表可以注册
int pass=0;				//pass=0代表两次输入密码一致
int	input_l=0;			//input_l=0代表输入信息正确

#define INVALID_USERINFO   'n'     //用户信息无效
#define VALID_USERINFO	   'y'     //用户信息有效

//获取用户输入存入到buf，buf的长度为len，用户输入数据以'\n'为结束标志

typedef struct th
{
	int conn_fd;
	char buf1[128];
	char username[32];
}THREAD;
/**
 *函数声明部分
 */
int userinfo_mat(char *f_username, char *s_username);
void my_register(int conn_fd);
void read_list();					//读取在线列表


int get_userinfo(char *buf, int len)
{
	int i;
	int c;
	int key=0;		//标志输入的是私聊还是群消息

	if(buf==NULL){
		return -1;
	}
	i=0;

	while( ( (c=getchar() ) !='\n') && (c !=EOF ) &&(i<len-2) ){
		buf[i++]=c;
		key=1;
	}
	buf[i++]='\n';
	buf[i++]='\0';

	return key;
}

char *input_userinfo(int conn_fd, const char *string)
{
	char input_buf[32];
	char recv_buf[BUFSIZE];
	char *username;
	char flag_userinfo;


	//输入用户信息直到正确为止
	do{
		printf("%s:",string);
		if( get_userinfo(input_buf, 32) <0 ){
			printf("error return from get_userinfo\n");
			exit(1);
		}
		if( send(conn_fd, input_buf, strlen(input_buf), 0) <0){
			my_err("send", __LINE__);
		}

		usleep(1000);
		if(input_l==1){
			input_l=0;
			flag_userinfo=VALID_USERINFO;
		}else{
			printf("%s error, input again", string);
			flag_userinfo=INVALID_USERINFO;
		}

	}while(flag_userinfo==INVALID_USERINFO);

	username=input_buf;
	return username;
}

void wchat_records(char *filename, char *records)
{
	int fd;		//文件描述符

	//检查，文件不存在则新建
	fd=open(filename, O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
	close(fd);

	//以追加的方式打开文件并写入
	if( (fd=open(filename, O_RDWR | O_APPEND) ) ==-1 ){
		my_err("fileopen", __LINE__);
	}

	if( write(fd, records, strlen(records)) != strlen(records) ){
		my_err("filewrite", __LINE__);
	}
	close(fd);
}
//接收数据的线程
void * thread2(int conn_fd)
{
	char buf1[128];;
	int  len;

	while(1){
	fflush(stdout);
	if( (len=recv (conn_fd, buf1, sizeof(buf1) ,0)  ) <0 ){
		my_err("recv",__LINE__);
	}

	//如果服务器端连接关闭
	 if(len==0){
		printf("no connection\n");
		close(conn_fd);
		exit(1);
		}

	//接受的是登录返回的消息则不输出
	else if(buf1[0]==VALID_USERINFO){
		input_l=1;
		printf("%s\n", buf1);
	}

	else{
		//接受的是私聊返回的消息
		if( strcmp(buf1, "Nonono\0") == 0){
			flag_key=0;								//flag_key==0代表消息发送对象不存在
		}
		//接受的是注册返回的消息
		else if( strcmp(buf1, "Username Already exists.Please choose another\0") ==0 ){
			flag_login=1;
		}
	
		//接受的是登录返回的消息
		else if(buf1[0]==VALID_USERINFO){
			input_l=1;
		}

		//输出接受到的消息
		printf("%s\n", buf1);
	}
	memset(buf1, 0, sizeof(buf1));

	}
}

//提取私聊接受数据的用户名
int message_pro(char *recv_buf, char *user ,char *info)
{
	int i, j=0;
	int str_len=0;
	int key=0;			//key=1代表找到@

	str_len=strlen(recv_buf);

	//找标志@
	for(i=0; i<str_len; i++){
		info[i]=recv_buf[i];
		if(recv_buf[i] == '@' ){
			key=1;
			break;
		}
	}
	info[i]='\0';
	for(j=0; j<32; j++){
		i++;
		user[j]=recv_buf[i];
		if(recv_buf[i] == '\0')
			break;
	}

	return key;
}

int userinfo_mat(char *f_username, char *s_username)
{
	FILE *fp;			//文件指针
	int ret;			//ret=0表示没有聊天记录
	char ch='a';				//遍历文件
	char name1[128];			//从文件获得的发送方名
	char name2[128];			//从文件获得接收方用户名
	int i,j, k;
	char buf[128];		//存一条消息记录
	int x=0,y=0;		//逻辑值


	fp=fopen("s_records", "rt");
	if(fp==NULL){
		my_err("fopen", __LINE__);
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
		while( (ch=fgetc(fp) ) != ':'){
		name2[j++]=ch;
		}
		name2[j]='\0';

		//找出内容
		x=  ( !(strcmp(f_username, name1) ) &&  !( strcmp(s_username, name2) ) );
		y=  ( !(strcmp(f_username, name2) ) &&  !( strcmp(s_username, name1) ) );
		if( x || y  ){
			while(  (ch=fgetc(fp)) != '_'){
				buf[k++]=ch;
			}
			buf[k]='\0';
			printf("%s@%s:%s\n", name1, name2, buf);
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
	fclose(fp);
}

void read_list()
{
	FILE *fp;
	char name[32];		//用来存储读取的用户名
	
	
	fp=fopen("list", "rt");
	if(fp==NULL){
		my_err("fopen", __LINE__);
	}
	while( fscanf(fp, "%s", name) != EOF){
		printf("%s\n", name);
	}
}
//发送数据的线程
void * thread1(THREAD *th)
{
	int thid=th->conn_fd;
	char recv_buf[128];
	char filename_s[32];				//私聊文件名
	char filename_q[32];				//群聊文件名

	char file_w[128];					//保存需要往文件里面存的一条消息
	int key=0;							//key=1代表存在@
	char acc[32];						//接受消息的用户名
	char info[128];						//纯消息
	char rec_name[32];					// 查聊天记录时的用户名

	strcpy(filename_s, "s_records");		//设置文件名
	strcpy(filename_q, "q_records");

	while(1){

		memset(recv_buf, 0, strlen(recv_buf));							//清零
		//输入消息内容
		if( ( key=get_userinfo(recv_buf, 128) ) <0 ){
			my_err("get_userinfo", __LINE__);
		}

		if( send(thid, recv_buf,strlen(recv_buf), 0 )<0 ){
		my_err("send", __LINE__);
		}

		//##代表获取聊天记录
		if( recv_buf[0] =='#' &&  recv_buf[1] == '#'){
			printf("输入要获取聊天记录的对象:\n");
			scanf("%s", rec_name);
			userinfo_mat(th->username, rec_name);
			continue;
		}

		//$$代表查看在线列表
		else if(recv_buf[0] == '*' && recv_buf[1] == '*'){
		//	read_list();
			continue;
		}
        key=message_pro(recv_buf, acc, info);
		acc[strlen(acc)-1]='\0';				//去掉\n

		usleep(100);

		//key==1代表存在@, flag_key==1代表@后的用户名存在
		if(key==1 && flag_key ==1 ){
			strcpy(file_w, "_\0");				//以_符号开头
			strcat(file_w, th->username);		//发消息的人

			strcat(file_w,"@\0");				
			strcat(file_w, acc);				//连接接消息的人

			strcat(file_w, ":\0");				//消息分割处
			strcat(file_w, info);				//连接消息
			strcat(file_w, "_\n\0");			//结尾并加换行符

			wchat_records(filename_s, file_w);	//
		}

		//写入文件
		else{
			wchat_records(filename_q, recv_buf);
		}

		flag_key=1;
		memset(file_w, 0, strlen(file_w));		//清零

		file_w[strlen(file_w)+1]='\0';
		file_w[0]='\0';
	}
}


//用户名注册函数
void my_register(int conn_fd)
{
	char press;							//注册时用
	char name[32];						//注册时输入的用户名
	char password[32];					//注册时输入的密码
	char password1[32];					//再次输入密码
	int  in_put=1;						//如果用户名存在考虑输入其他用户名
	int  stop=0;						//用于阻塞的变量到时候可能会用

	printf("Press y or Y to Register:");
	scanf("%c", &press);
	if(press == 'y' || press == 'Y'){
		while(in_put==1)
		{
			if( send(conn_fd, "register\0", strlen("register\0")+1 ,0) <0 ){
				my_err("send", __LINE__);
			}
			//发送用户名
			printf("\nPlease input a username:");
			scanf("%s", name);
			if( send(conn_fd, name, strlen(name)+1, 0) <0 ){
				my_err("send", __LINE__);
			}

			//停止一下下等待接受到用户是否存在的信息
			usleep(1000);
			//如果可以注册
			if(flag_login == 0){
				while(1)
				{
					printf("\nPlease input the password:");
					scanf("%s", password);
					printf("\nPlease input again:");
					scanf("%s", password1);

					//如果两次输入的密码匹配
					if( strcmp(password, password1) ==0 ){
						if( send(conn_fd, password, strlen(password)+1, 0) <0 ){
							my_err("send", __LINE__);
						}
						if( send(conn_fd, "achivement\0", strlen("achivement")+1, 0) <0 ){
							my_err("send", __LINE__);
						}
						break;
					}
					else{
						printf("two input password dose not match ,again\n");
					}	
				}
			}
			//将全局变量设置为可以注册的情况
		
			usleep(100);					//停一下下等待消息
			flag_login=0;							
			printf("press 1 register again:");
			scanf("%d", &in_put);
		}
	}


}

int main(int argc, char ** argv)
{

	int i;
	int ret;						
	int conn_fd;					
	int serv_port;
	struct sockaddr_in serv_addr;//创建套接字时用到的参数
	char recv_buf[BUFSIZE];		//缓冲区
	int thid1;					//线程号
	int thid2;
	THREAD *th;					//创建线程传参
	char	*string;				//用一个字符型指针
	

	th=(THREAD *)malloc(sizeof(THREAD));
	//检查参数个数
	if(argc != 5){
		printf("usage:asdfghasdfghjkl\n");
		exit(-1);
	}

	//初始化服务器端地址结构
	memset( &serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	//从命令行获取服务器端口与地址
	
	for(i=1; i<argc; i++){
		if(strcmp("-p", argv[i])==0){
			serv_port=atoi(argv[i+1]);
			if(serv_port<0 || serv_port>65535){
				printf("invalid serv_addr.sin_port\n");
				exit(1);
			}else{
				serv_addr.sin_port=htons(serv_port);
			}

			continue;
		}

		if(strcmp("-a", argv[i])==0){
			if(inet_aton(argv[i+1], &serv_addr.sin_addr)==0){
				printf("invalid server ip address\n");
				exit(1);
			}
			continue;
		}
	}

	//检测是否少输入了某项参数
	if(serv_addr.sin_port ==0 ||serv_addr.sin_addr.s_addr ==0 ){
		printf("asdfghjkl\n");
		exit(1);
	}

	//创建一个tcp套接字
	conn_fd=socket(AF_INET, SOCK_STREAM, 0);
	if(conn_fd <0 ){
		my_err("socket", __LINE__);
	}

	//向服务器端发送连接请求
	if(connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) <0 ){
		my_err("connect", __LINE__);
	}

	//创建一个线程接受数据
	pthread_create(&thid2, NULL, (void*)thread2, conn_fd);
	//用户注册
        my_register(conn_fd);
		printf("======================\n");
		getchar();


	//用户登录部分输入用户名和密码
	string=input_userinfo(conn_fd, "username");

	strcpy(th->username, string);
	th->username[strlen(th->username)-1]='\0';		//去掉'\n' 获取用户名

	input_userinfo(conn_fd, "password");
	
	//for(i=0; i<ret; i++){
	//	printf("%c",recv_buf[i]);
//	}

	//创建一个线程发送数据
	th->conn_fd=conn_fd;
	pthread_create(&thid1, NULL, (void *)thread1, th);
	
	while(1){
		sleep(1);
	}
	close(conn_fd);
	return 0;
	
}
