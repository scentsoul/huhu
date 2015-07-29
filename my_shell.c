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
#include<time.h>
#include<time.h>
#include<netinet/in.h>
#include<errno.h>
#include<fcntl.h>
#include<dirent.h>
#include<linux/limits.h>
#include<grp.h>
#include<pwd.h>
#include<sys/wait.h>

#define normal             0          /*一般的命令*/
#define out_redirect       1          /*输出重定向*/ 
#define in_redirect        2          /*输入重定向*/
#define have_pipe		   3		  /*命令中有管道*/

void print_prompt();
void get_input(char *buf);
void explain_input(char *buf, int *argcount, char arglist[100][256]);
void do_cmd(int argcount, char arglist[100][256]);
int find_command(char * command);

void print_prompt()
{
	printf("my_shell$$ ");
}

/*获取用户输入*/
void get_input(char *buf)
{
	int len=0;
	int ch;   /*改动*/

	ch=getchar();
	while(len<256 && ch != '\n'){
		buf[len++]=ch;
		ch=getchar();
	}

	if(len==256){
		printf("command is too long\n");
		exit(-1);
	}
	buf[len]='\n';  /*改动*/
	len++;
	buf[len]='\0';
}
/*解析buf中的命令，将结果存入arglist中，命令以回车符号\n结束*/
/*例如输入命令为 "ls -l /temp" 则arglist[0].arglist[1].arglist[2],分别为 ls -l tm p*/
void explain_input(char *buf, int *argcount, char arglist[100][256])
{
	char *p=buf;
	char *q=buf;
	int number=0;

	while(1){
		if(p[0] == '\n')
			break;
		if(p[0] == ' ')
			p++;
		else{
			q=p;
			number=0;
			while( q[0] != ' ' && q[0] != '\n'){
				number++;
				q++;
			}
			strncpy(arglist[*argcount], p, number+1);
			arglist[*argcount][number]='\0';
			*argcount=*argcount+1;    /*改动*/
			p=q;
		}
	}
}

void select1(int how, pid_t pid, char *arg[], char * file, char * argnext[])
{

	int fd;
	switch(how){
		case 0:           //pid为０说明是子进程，在子进程中执行输入的命令
			if(pid==0){
				if( !(find_command(arg[0]))){
					printf("%s :command not find\n", arg[0]);
				    exit(0);
			}
				execvp(arg[0] , arg);
			    exit(0);
			}
			break;
		case 1:	
			if(pid==0){
				if( !(find_command(arg[0]))){
					printf("%s :command not find\n", arg[0]);
				    exit(0);
				}
			   fd=open(file, O_RDWR|O_CREAT|O_TRUNC, 0644);
			   dup2(fd,1);
			   execvp(arg[0], arg);
			   exit(0);
			}
			break;
		case 2:
			if(pid==0){
				if( !(find_command(arg[0]))){
					printf("%s :command not find\n", arg[0]);
				    exit(0);
				}
			   fd=open(file, O_RDONLY);
			   dup2(fd,0);
			   execvp(arg[0], arg);
			   exit(0);
			}
			break;
		case 3:
			if(pid==0){
				int pid2;
				int status2;
				int fd2;

				if( (pid2==fork() ) <0 ){
					printf("fork2 error\n");
					return ;
				}
				else if(pid2==0){
				if( !(find_command(arg[0]))){
					printf("%s :command not find\n", arg[0]);
				    exit(0);
				}

				fd2=open("/tmp/youdonotknowfile",O_WRONLY|O_CREAT|O_TRUNC,0644);
				dup2(fd2,1);
				execvp(arg[0], arg);
				exit(0);
			}
			if(waitpid (pid2, &status2,0)==-1){
				printf("wait for child process error\n");
			}
			if( !(find_command(argnext[0])) ){
				printf("%s :command not find\n", arg[0]);
				exit(0);
			}

			fd2=open("/tmp/youdonotknowfile",O_RDONLY);
			dup2(fd2, 0);
			execvp(argnext[0], argnext);
			if(remove ("/tmp/youdonotknowfile"))
				printf("remove error\n");
			exit(0);
			}
			break;
		default:
			break;
	}
}
void do_cmd(int argcount, char arglist[100][256])
{
	int flag=0;
	int how=0;             // 用于指示命令中是否有>、<、|
	int background=0;       //标识命令中是否有后台运行标识符 &
	int status;
	int i;
	int fd;
	char *arg[argcount+1];        //改动
	char *argnext[argcount+1];    //改动
	char *file;
	pid_t pid;
	//将命令取出
	for(i=0; i<argcount; i++){
		arg[i]=(char *)(arglist[i]);
	}

	arg[argcount]=NULL;

	for(i=0; i<argcount; i++){
		if( strncmp(arg[i], "&", 1) == 0 ){
			if(i==argcount-1){
				background=1;
				arg[argcount-1]=NULL;
				break;
			}
			else{
				printf("wrong commond\n");
				return ;
			}
		}
	}

	for(i=0; arg[i] != NULL; i++){
		if(strcmp(arg[i], ">") ==0 ){
			flag++;
			how=out_redirect;
			if(arg[i+1]==NULL)
				flag++;
		}
		if( strcmp(arg[i], "<") ==0 ){
			flag++;
			how=in_redirect;
			if(i==0)
				flag++;
		}

		if( strcmp(arg[i],"|")==0 ){
			flag++;
			how=have_pipe;
			if(arg[i+1] ==NULL)
				flag++;
				if(i==0)
					flag++;
		}
	}

	//flag  大于1，说明命令中含有多><| 符号，或者格式不对，本程序不支持这样的命令

	if(flag>1){
		printf("wrong command\n");
		return;
	}
	if(how==out_redirect  || how==in_redirect) {
		for(i=0; arg[i] != NULL; i++){
			if( strcmp(arg[i],">")==0  || strcmp(arg[i], "<")==0) {//命令只含有一个输入重定向符
				file=arg[i+1];
				arg[i]=NULL;
			}
		}
	}

	if(how==have_pipe){
		for(i=0; arg[i] != NULL; i++){
			if(strcmp(arg[i], "|")==0){
				arg[i]==NULL;
				int j;                      //gai
				for(j=i+1; arg[j] != NULL; j++){
					argnext[j-i-1]=arg[j];
				}

				argnext[j-i-1]=arg[j];
				break;
			}
		}
	}

	if((pid=fork()) <0 ){
		printf("fork error\n");
		return;
	}



select1(how, pid, arg, file, argnext);    /*选择*/

	//若命令中有＆，表示后台执行，父进程直接返回，不等待子进程结束
	if( background ==1){
		printf("prcess id %d\n", pid);
		return;
	}
	//父进程等待子进程结束
	if(waitpid(pid ,&status, 0)==-1)
		printf("wait for child process error\n");
}
	//查找命令中可执行程序
	int find_command(char * command)
	{
		DIR *dp;
		struct dirent * dirp;
		char *path[]={"./","/bin","/usr/bin",NULL};
		/*使当前目录下的程序可以运行*/
		if( strncmp(command,"./", 2) ==0 )
			command=command+2;

		int i=0;
		while(path[i] !=NULL ){
			if( (dp=opendir(path[i])) ==NULL )
				printf("can not open /bin\n");
			while(  (dirp = readdir(dp)) !=NULL ){
				if(strcmp(dirp->d_name,command)==0){
					closedir(dp);
					return 1;
				}
			}

			closedir(dp);
			i++;
		}
		return 0;
	}
	
int main(int argc, char* argv[], char **environ)
{
	int i;
    int argcount=0;
	char arglist[100][256];
	char **arg=NULL;
	char *buf=NULL;
	buf =(char *)malloc(256);
	if(buf==NULL){
		printf("malloc error\n");
		exit(-1);
	}

	while(1){
		memset(buf, 0,256);
		print_prompt();
		get_input(buf);
		if( strcmp(buf,"exit\n")==0 || strcmp(buf, "logout\n")==0){
			break;
		}
		for(i=0; i<100; i++){
			arglist[i][0]='\0';
		}
		argcount=0;
		explain_input(buf, &argcount, arglist);
		do_cmd(argcount, arglist);
	}
	if( buf != NULL){
		free(buf);
		buf=NULL;
	}
	printf("haha\n");
	exit(0);
}
