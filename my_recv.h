/*************************************************************************
    > File Name: my_recv.h
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年08月06日 星期四 00时05分59秒
 ************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#ifndef _MY_RECV_H
#define _MY_RECV_H
     #define BUFSIZE 1024
	 void my_err(const char * err_string, int line);
	 int my_recv(int conn_fd, char *data_buf, int len);
#endif
