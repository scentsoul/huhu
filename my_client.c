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
#include"my_recv.h"

#define INVALID_USERINFO   'n'     //用户信息无效
#define VALID_USERINFO	   'y'     //用户信息有效

//获取用户输入存入到buf，buf的长度为len，用户输入数据以'\n'为结束标志

typedef struct th
{
	int conn_fd;
	char buf1[128];
}THREAD;

int get_userinfo(char *buf, int len)
{
	int i;
	int c;

	if(buf==NULL){
		return -1;
	}
	i=0;

	while( ( (c=getchar() ) !='\n') && (c !=EOF ) &&(i<len-2) ){
		buf[i++]=c;
	}
	buf[i++]='\n';
	buf[i++]='\0';

	return 0;
}

void input_userinfo(int conn_fd, const char *string)
{
	char input_buf[32];
	char recv_buf[BUFSIZE];
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
		//从套接字上读取一次数据
		if( my_recv(conn_fd, recv_buf, sizeof(recv_buf)) <0 ){
			printf("data is too long\n");
			exit(1);
		}

		if(recv_buf[0]==VALID_USERINFO){
			flag_userinfo=VALID_USERINFO;
		}else{
			printf("%s error, input again", string);
			flag_userinfo=INVALID_USERINFO;
		}

	}while(flag_userinfo==INVALID_USERINFO);
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
	else if(len==0){
		printf("no connection\n");
		close(conn_fd);
		exit(1);
		}

	printf("%s\n", buf1);
	memset(buf1, 0, sizeof(buf1));
	}
}

//发送数据的线程
void * thread1(THREAD *th)
{
	int thid=th->conn_fd;
	char recv_buf[128];

	while(1){
		if(get_userinfo(recv_buf, 128) <0 ){
			my_err("get_userinfo", __LINE__);
		}
		if( send(thid, recv_buf,strlen(recv_buf), 0 )<0 ){
		my_err("send", __LINE__);
		}

	}
}
int main(int argc, char ** argv)
{
	int i;
	int ret;
	int conn_fd;
	int serv_port;
	struct sockaddr_in serv_addr;
	char recv_buf[BUFSIZE];
	int thid1;
	int thid2;
	THREAD *th;

	int status;					//用于join等待时的状态变量

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

	//输入用户名和密码

	input_userinfo(conn_fd, "username");
	input_userinfo(conn_fd, "password");

	//读取欢迎信息并打印
	if( (ret=my_recv(conn_fd, recv_buf, sizeof(recv_buf)))  <0){
		printf("data is too long\n");
		exit(1);
	}
	for(i=0; i<ret; i++){
		printf("%c",recv_buf[i]);
	}

	fflush(stdout);
	printf("\n");
	//创建一个线程接受数据
	pthread_create(&thid2, NULL, (void*)thread2, conn_fd);

	/*if(get_userinfo(recv_buf, 128) <0 ){
		my_err("get_userinfo", __LINE__);
	}*/


	//创建一个线程发送数据
	th->conn_fd=conn_fd;
	//strcpy(th->buf1, recv_buf);	//赋值以便发送数据
	pthread_create(&thid1, NULL, (void *)thread1, th);


	//pthread_join(thid1, (void *)&status);
	//sleep(50);


	while(1){

		sleep(1);
	}
	close(conn_fd);
	return 0;
	
	//pthread_exit(0);
}
