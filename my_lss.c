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
#define PARAM_NONE 0   //无参数
#define PARAM_A 1      //-a :显示所有文件
#define PARAM_L 2      //-l：一行只显示一个文件的信息
#define MAXROWLEN 80   //一行显示的最多字符数

int g_leave_len=MAXROWLEN; //一行剩余长度，用于输出对齐
int g_maxlen;              //存放某目录下最长文件名的长度

struct tem
{
	char dirnames[80];
	struct tem *next;
};                             //定义一个结构体保存目录





/*函数声明部分*/
void display_attribute(struct stat buf, char *name);   //获取属性并打印函数
void display_single(char *name ,int sT_INO, struct stat buf); //打印文件名
void display(int flag, char name[], int  sT_INO);      //选择，case
void display_dir(int flag_param, char *path, int time, int file,int sT_INO); //操作目录



/*错误处理函数，打印出错误所在行数和错误信息*/
void my_err(const char *err_string, int line)
{
	fprintf(stderr, "line: %d",line);
	perror(err_string);
	exit(0);
}
/*获取文件属性并打印*/

void display_attribute(struct stat buf, char *name)
{
	char buf_time[32];
	struct passwd *psd;   //从改结构体获取文件所有者的用户名
	struct group *grp;    //从该结构体中获取文件所有者所属组的组名
	
	/*获取并打印文件类型*/

	if(S_ISLNK (buf.st_mode) ){         //是否为符号链接
		printf("l");
	}
	else if(S_ISREG(buf.st_mode) ){      //是否为一般文件
		printf("-");
	}
	else if(S_ISDIR(buf.st_mode) ){     //目录文件
		printf("d");
	}
	else if(S_ISCHR(buf.st_mode) ){     //字符设备文件
		printf("c");
	}
	else if(S_ISBLK(buf.st_mode) ){     //块设备文件
		printf("b");                 
	}
	else if(S_ISFIFO(buf.st_mode) ){    //是否为先进先出FIFO
		printf("f");       
	}
	else if(S_ISSOCK(buf.st_mode) ){
		printf("s");
	}
	/*获取并打印文件所有者权限*/
	if(buf.st_mode & S_IRUSR){
		printf("r");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IWUSR){
		printf("w");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IXUSR){
		printf("x");
	}else{
		printf("-");
	}
	/*获取并打印与文件所有者同组的用户对该文件的操作权限*/
	if(buf.st_mode & S_IRGRP){
		printf("r");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IWGRP){
		printf("w");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IXGRP){
		printf("x");
	}else{
		printf("-");
	}
	/*获取并打印其他用户对该文件的操作权限*/
	if(buf.st_mode & S_IROTH){
		printf("r");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IWOTH){
		printf("w");
	}else{
		printf("-");
	}
	if(buf.st_mode & S_IXOTH){
		printf("x");
	}else{
		printf("-");
	}

	printf("   ");
	/*根据uid与gid获取文件所有者的用户名与组名*/
	psd=getpwuid(buf.st_uid);
	grp=getgrgid(buf.st_gid);
	printf("%4d", (int)(buf.st_nlink) );  /*打印文件的链接数*/
	printf("%-8s",psd->pw_name);
	printf("%-8s",grp->gr_name);

	printf("%6d",(int)(buf.st_size));   //打印文件的大小
	strcpy(buf_time, ctime(&buf.st_mtime));
	buf_time[strlen(buf_time)-1]='\0';
	printf(" %s",buf_time);
}

/*在没有使用-l选项时，打印一个文件名，打印时上下行对齐*/
void display_single(char *name ,int sT_INO, struct stat buf)   //sT_INO用于判断有没有-i
{
	int i,len;
	/*如果本行不足以打印一个文件名则换行*/
	if(g_leave_len < g_maxlen){
		printf("\n");
		g_leave_len=MAXROWLEN;
	}
	len=strlen(name);
	len=g_maxlen-len;
	if(sT_INO==1){
	printf("%d ",(int)(buf.st_ino));                   //sT_INO为1的时候用于-i
	}
	printf("%-s",name);

	for(i=0; i<len+6; i++){
		printf(" ");
	}

    printf("      ");
	g_leave_len -= (g_maxlen+2);
}
/*根据命令行参数和完整路径名显示目标文件
 * 参数flag：命令行参数
 * 参数pathname：包含了文件名的路径名*/

void display(int flag, char pathname[], int  sT_INO)     //是不是-i,name是文件名
{
	int          i,j;
	struct stat buf;
        char         name[NAME_MAX+1];

	/*从路径中解析出文件*/
	for(i=0,j=0; i<strlen(pathname); i++){
		if(pathname[i]=='/'){
			j=0;
			continue;
		}

		name[j++]=pathname[i];
	}
	name[j]='\0';
	/*用lstat而不是stat以方便解析链接文件*/

if( lstat(pathname, &buf) ==-1 ){
		my_err("stat",__LINE__);
	}
	switch(flag){
		case PARAM_NONE:               //没有-l和-a选项
			if(name[0] != '.'){
				display_single(name,sT_INO,buf);
			}
			break;
		case PARAM_A:                 //-a显示包括隐藏文件在内的所有文件
			display_single(name, sT_INO, buf);
			break;
		case PARAM_L:                //-l每个文件单独占一行，显示文件详细属性信息
			if(name[0] != '.'){
				display_attribute(buf,name);
				printf("  %-s\n",name);
			}

			break;
		case PARAM_A+PARAM_L:        //同时有-a和-l的选项
			display_attribute(buf,name);
			printf("  %-s\n",name);
			break;
		default:
			break;
	}
}
void sort(char a[256][PATH_MAX+1],int left, int right)  //字符串排序
{
	int i=left;
	int j=right;
	char key[20];

	strcpy(key, a[left]);

	if(left >= right){
		return ;
	}

	while( i< j )
	{
		while(i<j && strcmp(key, a[j])<0 ){
			j--;
		}
		strcpy( a[i], a[j] );

		while(i<j && strcmp(a[i] , key)<=0 ){
			i++;
		}

	    strcpy( a[j], a[i] );
	}

	strcpy(a[i], key);
    sort(a, left, i-1);
	sort(a, i+1, right);
}
void sort_time(char a[256][PATH_MAX+1], int left, int right,char mm[256][PATH_MAX])
{
	int i=left;
	int j=right;
	char key[80];
	char key1[80];

	strcpy(key,mm[left]);
	strcpy(key1,a[left]);

	if(left >= right){
		return ;
	}

	while( i<j ){
		while( i<j && strcmp(key, mm[j]) <0){
			j--;
		}

		strcpy(a[i], a[j]);
		strcpy(mm[i],mm[j]);

		while(i<j && strcmp(mm[i], key) <=0){
			i++;
		}
		strcpy(a[j], a[i]);
		strcpy(mm[j],mm[i]);
	}

	strcpy(mm[i], key);
	strcpy(a[i],key1);
	sort_time(a, left, i-1, mm);
	sort_time(a, i+1, right, mm);
}
char comp(char *m)
{
	if(  strcmp(m,"Jan\0")==0 )
		return 'a';
	else if(  strcmp(m,"Feb\0")==0 )
		return 'b';
	else if(  strcmp(m,"Mar\0")==0 )
		return 'c';
	else if(  strcmp(m,"Apr\0")==0 )
		return 'd';
	else if(  strcmp(m,"May\0")==0 )
		return 'e';
	else if(  strcmp(m,"Jun\0")==0 )
		return 'f';
	else if(  strcmp(m,"Jul\0")==0 )
		return 'g';
	else if(  strcmp(m,"Aug\0")==0 )
		return 'h';
	else if(  strcmp(m,"Sep\0")==0 )
		return 'i';
	else if(  strcmp(m,"Oct\0")==0 )
		return 'j';
	else if(  strcmp(m,"Nov\0")==0 )
		return 'k';
	else if(  strcmp(m,"Dec\0")==0 )
		return 'l';
	else return 'z';                            //按月份返回
}
void update(int i, char mm[256][PATH_MAX], char hh[256][PATH_MAX])
{
	int j=0,k=0;
	char pre[4];
	for(j=0; j<3; j++){
		pre[j]=mm[i][j+4];
	}
	pre[3]='\0';
	j=strlen(mm[i])-4;

	for(k=0; k<4; k++){
		hh[i][k]=mm[i][j+i];

	}
	hh[i][k++]=comp(pre);
	for(j=8; j<19; j++){
		hh[i][k++]=mm[i][j];
	}

	hh[i][k]='\0';                       //排好时间格式
	
}
/*time=1表示按时间排序，file=1表示按文件名正=2反，sT_INO表示是否有-i*/
void display_dir(int flag_param, char *path, int time, int file,int sT_INO)
{
	DIR		*dir;
	struct  dirent  *ptr;
	int     count=0,i,j,len;
	char    filenames[256][PATH_MAX+1],temp[PATH_MAX+1];
	struct stat buf;
	char mm[256][PATH_MAX];
	char hh[256][PATH_MAX];
	char buf_time[32];         /*存时间*/
     
	/*获取该目录下文件总数和最长的路径名*/
	dir=opendir(path);
	if(dir==NULL){
		my_err("opendir",__LINE__);
	}
	while( (ptr=readdir(dir)) !=NULL ){
		if(g_maxlen < strlen(ptr->d_name))
			g_maxlen=strlen(ptr->d_name);
		count++;
	}
	closedir(dir);
	len=strlen(path);
	if(count>256){
		my_err("readdir",__LINE__);
	}
    //获取该目录下所有的文件名
	dir=opendir(path);
	for(i=0; i<count; i++){
		ptr=readdir(dir);
		if(ptr==NULL){
			my_err("readdir", __LINE__);
		}
	strncpy(filenames[i],path,len);
	filenames[i][len]='\0';
	strcat(filenames[i], ptr->d_name);
	filenames[i][len+strlen(ptr->d_name)]='\0';
	}



	for(i=0 ;i<count; i++){
		j=0;
		if(lstat(filenames[i], &buf)==-1){
			my_err("stst",__LINE__);
		}                                    //将文件文件解析，时间保存在mm里面
		strcpy(buf_time, ctime(&buf.st_mtime) );
		buf_time[strlen(buf_time)-1]='\0';  // qudiaohuanhang
		strcpy(mm[i], buf_time);


	//	update(i, hh, mm);                 //将mm按指定方式保存在hh里面用于排序
	//	printf("%s",hh[i]);
	//	getchar();
	}



	if(time==1){
		sort_time(filenames, 0, count-1, mm);
		for(i=0; i<count; i++){
		display(flag_param, filenames[i],sT_INO);
		}//将排序好的文件输出
	return ;	
	}





	else{
	if(file !=1 && file !=2){
		for(i=0; i<count; i++)
		display(flag_param, filenames[i],sT_INO);
	}
	else
		sort(filenames,0, count-1);           //按文件名排序
	if(file==1){
		for(i=0; i<count; i++)
		display(flag_param, filenames[i],sT_INO);
	}                                    //正输     
	if(file==2){
		for(i=count-1; i>=0; i--){
			display(flag_param, filenames[i], sT_INO);
		}
	}                                    //反输

	}
	closedir(dir);

	/*如果命令行中没有-l选项，打印一个换行符*/
	if( (flag_param & PARAM_L)==0)
		printf("\n");
}

void R_rmode(int flag_param, char *path)
{
	struct stat buf;
	struct tem dianame;
	if(   stat(path, &buf)== -1 ){
		my_err("stat", __LINE__);
	}


}
	
int main(int argc, char ** argv)
{

	
   	int i, j=0, k, num=0;
	char path[PATH_MAX+1];
	char param[32];            //保存命令行参数，目标文件和目录名不在此列
	int flag_param=PARAM_NONE;  //参数种类，即是否有-l和-a存在
	struct stat buf;
	int time=0;
	int file=0;
	int sT_INO=0;
    
	for(i=1; i<argc; i++){
		if(argv[i][0]=='-'){
			for(k=1; k<strlen(argv[i]); k++, j++){
				param[j]=argv[i][k];  //获取-后面的参数保存到param中
			}

			num++;                    //保存-的个数
		}
	}

    printf("haahah\n");
	
	//只支持参数a和l如果有其他参数就报错
	for(i=0; i<j; i++){
		if(param[i]=='a'){
			flag_param |= PARAM_A;
			continue;
		}else if(param[i]=='l'){
			file=1;
			flag_param |= PARAM_L;
			continue;

		}else if(param[i]=='i'){
			sT_INO=1;
			continue;
		}else if(param[i]=='t'){
			time=1;
          continue;
		}
		else if(param[i]=='r'){
			file=2;
		}
		else{
			printf("my_ls:invalid optio -%c\n", param[1]);
			exit(1);
		}
	}

	param[j]='\0';
	//如果没有输入文件名或目录就显示当前目录
	if( argc==2){
		strcpy(path, "./");
		path[2]='\0';
		display_dir(flag_param, path,time, file, sT_INO );

		return 0;
	}
	i=1;
	do{//如果不是目标文件或目录，解析下一个命令参数
		if(argv[i][0]=='-'){
			i++;
			continue;
		}else{
			strcpy(path, argv[i]);
			//如果目标文件或目录不存在，则报错并退出程序
			if(  stat(path, &buf)==-1  ){
				my_err("stat",__LINE__);
			}
			if(S_ISDIR(buf.st_mode)){  //argv[i]是一个目录
				                       //如果目录的最后一个字符不是'/'就加上'/'
				if(  path[strlen(argv[i])-1] != '/'){
					path[strlen(argv[i])]='/';
					path[strlen(argv[i])+1]='\0';
				}
				else 

					path[ strlen(argv[i])] ='\0';
				display_dir(flag_param, path,time, file, sT_INO);
				i++;
			}else{   //argv[i]是一个文件
				display(flag_param, path,sT_INO);
				i++;
			}
		};

	}while(i<argc);

	return 0;
}
