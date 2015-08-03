/*************************************************************************
    > File Name: test_lock.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年07月31日 星期五 10时22分15秒
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
#include<pthread.h>
pthread_mutex_t  number_mutex;
int globalnumber=0;
void * write_globalnumber()
{

    //pthread_mutex_lock(&number_mutex);
	globalnumber += 1;
	globalnumber += 1;
	globalnumber += 1;
	sleep(1);                       //为了展示锁的效果停顿一秒
	globalnumber += 1;
	globalnumber += 1;
	globalnumber += 1;
	printf("%da\n",globalnumber);
    //pthread_mutex_unlock(&number_mutex);

	printf("b\n");
	pthread_exit(0);         //线程退出
}

void* read_globalnumber()
{
	int tmp;
    pthread_mutex_lock(&number_mutex);        //锁住
	tmp=globalnumber;
	printf("%dc\n",tmp);
    pthread_mutex_unlock(&number_mutex);//解锁

	pthread_exit(0);
}
int main(void)
{
	pthread_t thid1;
	pthread_t thid2;
	int tmp;

	pthread_mutex_init(&number_mutex, NULL);   //初始化锁
	printf("start\n\n");
	pthread_create(&thid1, NULL, (void *)write_globalnumber, NULL);
	pthread_create(&thid2, NULL, (void *)read_globalnumber, NULL);

	sleep(1);
	tmp=globalnumber;
	printf("%d\n",tmp);


	sleep(5);
	printf("\n\n");
	return 0;
}

