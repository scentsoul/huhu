/*************************************************************************
    > File Name: 2.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年07月21日 星期二 16时14分53秒
 ************************************************************************/
#include<assert.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include<utime.h>
#include<time.h>
#include<netinet/in.h>
#include<errno.h>
#include<fcntl.h>
#include<dirent.h>
#include<linux/limits.h>
#include<grp.h>
#include<pwd.h>
#include<pthread.h>
pthread_key_t key;
void * thread2(void *arg)
{
	int tsd=5;
	int status;
	printf("thread %u is running\n",(unsigned int)(pthread_self()) );
	pthread_setspecific(key, (const void *)(&tsd));
	printf("thread %u returns %u\n",(unsigned int)(pthread_self()), *( (int*)(pthread_getspecific(key)))  );
	pthread_exit(0);
}

void *thread1(void *arg)
{
	int tsd=0;
	pthread_t thid2;


	printf("thread %u is running\n",(unsigned int)(pthread_self()) );
	pthread_setspecific(key, (void *)(&tsd));
	pthread_create(&thid2, NULL, thread2, NULL);
	sleep(3);
	printf("thread %u returns %d\n",(unsigned int)(pthread_self()), *( (int*)(pthread_getspecific(key)))  );
	pthread_exit(0);
}
int main(void)
{
	pthread_t thid1;
	int status;
	printf("\n\nmain thread begins running\n");
	pthread_key_create(&key, NULL);
	pthread_create(&thid1, NULL ,thread1, NULL);
	sleep(5);
	pthread_key_delete(key);
	printf("main thread exit\n");
	return 0;
}
