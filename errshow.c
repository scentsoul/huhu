/*************************************************************************
    > File Name: errshow.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年08月02日 星期日 15时10分57秒
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
#include<errno.h>
FILE *open_file(char * filename)
{
	FILE *stream;
	errno=0;

	stream=fopen(filename, "r");        //改变了errno的值
	if(stream==NULL)
	{
		perror(filename);　　　　　　　//根据errno打印出对应的错误提示信息
		//printf("can not open the file %s reason :%s\n",filename,strerror(errno));
		exit(-1);
	}

	else 
		return stream;
}

int main(void)
{
	char *filename="test";
	open_file(filename);
	return 0;
}
