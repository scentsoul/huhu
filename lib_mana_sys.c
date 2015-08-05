/**************************************************************************
    > File Name: test2.c
    > Author: huxingju
    > Mail: 104046058@qq.com 
    > The compiler environment:vim + g++
    > Created Time: 2015年07月15日 星期三 21时27分17秒
 ************************************************************************/
#include<stdio.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/socket.h>
#include<termios.h>
#include<sys/types.h>
#include<string.h>
#include<time.h>
/***********************************
 *宏定义部分
************************************/
#define high 8
#define WD   15
#define MAXSTAR 100   //字母总个数
#define ZWD 64        //界面宽度
#define COR 10
#define HIGH 3
#define N   50

/************************************
 * 定义结构体
 ************************************/
struct STAR
{
	int    x;
	int    y;
	int    step;
	int    color;
};
struct STAR star[MAXSTAR];

typedef struct data
{
	int year;
	int month;
	int day;

}DATA;                   //出版日期
struct STU
{
	char num[N];    //学号
	int flag;       //借书状态，1代表借，0代表没借，-1代表超时
        int day;        //借书的天数
};                      //学生信息
typedef struct adm
{
	char account[N];
	char password[N];
}ADM;                    //管理员信息

typedef struct book
{
	char num[N];            //图书编号
	char bname[N];          //书名
	char wname[N];          //作者姓名
	int price;               //价格
	int number;              //数量
	DATA publish;            //出版日期
	int flag;                //是否借出  //1借出０没有
	struct book *next;
	struct book *next1;      //图书结构体


	struct STU student;       //定义一个学生类型变量
	struct adm admini;               //定义一个管理者变量
}BOOK;                  //结构体  

/* **********************************
  *函数声明部分
 ************************************/

void changemode(int dir);      //模式设置函数
int  face ();                  //选择界面
int kbhit(void);               //类似于windows下的kbhit函数
void movestar(int i);          //星星移动
void initstar(int i);          //初始化星星
void line(int i, int n, int x,int y);  //画线
void drawbox();                        //画框
int clear1();                          //自定义清屏函数
int home();                            //最初界面
int in_selection(int x, int y);        //控制输入数字在指定范围内
BOOK *insert();                        //图书信息录入所需结点函数
BOOK* instu();                         //学生信息录入所需结点函数
BOOK *in_admini();                     //管理员信息录入所需结点函数
void free1(BOOK *head);                //释放链表
BOOK * creat(int i);                   //创建一个带头结点的单链表用于录入信息
int print(BOOK* head, int i,int j,int m) ;               //输出链表，ｍ代表输出类型
int borrface();                         //借书者界面
void save_inf(BOOK *head,int n);        //从文件中读取函数
int func(char *a, char *b, int count1,int count2);  // 统计英文字母匹配度最高数
int admi();                             // 管理员选择界面
int check();                            //登录界面
int login_check(char account[],char pass[],int n);  //核对账户和密码
int query();                             //各种查询
void compare(char a[],int k);            //精确查询所需要的比较
BOOK *insert1(BOOK *pre);                //精确查找所需函数
BOOK *PI(BOOK *head);                    //排序
BOOK* ass(BOOK *p);			             //结点保存值
int admi_return();                       //管理员将书入库
BOOK *del(BOOK * head,char a[]);         //删除链表中的一个结点
BOOK * fuzzy_chinese();                   //汉语书名模糊查询
int student_take(int i);                 //学生借还书
int take_out();                          //管理员取书
/***************************************
 * 以下是各功能函数原型
 ****************************************/
void initstar(int i)
{
	star[i].x=WD;
	star[i].y=rand()%30+high;
	star[i].step=rand()%5+1;           //用来表示向右移动步数
	star[i].color=rand()%9+30;         //30---39表示颜色
}
void movestar(int i)
{
	int n;
    n=rand()%26+97;                   //产生随机字母
	printf("\033[%d;%dH\033[?25l",star[i].y,star[i].x);
	if(n%26==0)
		printf("\033[1m\033[37m.");         
	if(n%56==0)
		printf("\033[30m.");         
	else
		printf("\033m  ");             //清除字母*/
	star[i].x += star[i].step;
	if(star[i].x>64+WD)
		initstar(i);

	printf("\033[%d;%dH\033[?25l",star[i].y,star[i].x);     //移动并隐藏光标
    printf("\033[%dm%c\033[1m ",star[i].color,n);          //显示数字及其颜色
} 
void changemode(int dir)                                    //模式设置函数
{
	static struct termios oldt,newt;
	if(dir==1)
	{		tcgetattr( STDIN_FILENO,&oldt);
		newt=oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
	}
	else
		tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
}
int kbhit(void)                                             //类似于windows下的kbhit 函数
{
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);
	select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO, &rdfs);
}
int  home()
{
    int i;
	changemode(1);                    //打开模式
	srand( (unsigned) time (NULL));  //随机种子
	for(i=0; i<MAXSTAR; i++)
	{
		initstar(i);
		star[i].x=rand()%64+WD;          //初始化位置和字母
	}
	while(!kbhit())
	{
		for(i=0; i<MAXSTAR; i++)
	        movestar(i);
		  printf("\033[4;3HPress Enter key to start:....");
		  sleep(1);

	}                                //控制按钮
    changemode(0);
	printf("\033[?25h");             //将光标显示出来
	return 0;
}
void line(int i, int n, int x,int y)
{
	int j;
	if(i==1){                      //1，画横线,n代表长度
	   for(j=0; j<n; j++){
		   printf("\033[%d;%dH",y,x+j);
	       printf("\033[35m\033[1m=");}
	}
	   if(i==2){
		for(j=0; j<n; j++){
			printf("\033[%d;%dH",y+j,x);
	    	printf("\033[37m.");}
	}
}                                 //画线
void drawbox()
{
	line(1,55,COR,HIGH);
	line(2,20,COR,HIGH);
	line(1,55,COR,HIGH+20);
	line(2,21,COR+55,HIGH);
	line(1,54,COR+1,HIGH+2);
    printf("\033[%d;%dH",HIGH+1,COR+5);
	printf("\033[36mwelcome to the library management system!");

}                                //画框

int in_selection(int x, int y)
{
	int i;
	
    fflush(stdin);
	drawbox();
    printf("\033[%d;%dH",HIGH+18,COR+20);	
	printf("Choose:");
    scanf("%d",&i);
    if(i<x || y<i)
	{
		fflush(stdin);  //清缓存
	    clear1();
            printf("\033[%d;%dH",HIGH+19,COR+20);
	    printf("error:input again");
            printf("\033[%d;%dH",HIGH+20,COR+41);
                          //循环输入要选择的功能，知道输入有效功能编号
	}
	return i;
}
int face()
{
	int i;
	while(1)
	{
         printf("\033[%d;%dH",HIGH+5,COR+10);
	    printf("1.Borrower (借书的人)");
	    printf("\033[%d;%dH",HIGH+10,COR+10);
	    //fflush(stdin);              
	    printf("2.Administrator (管理员)");
	    i=in_selection(1,2) ;
		if(i==1 || i==2)
         	return i;
	}
}                          //选择身份界面(主页)
int borrface()
{
	int i;
	while(1){
		drawbox();
        printf("\033[%d;%dH",HIGH+3,COR+10);
	    printf("1.Query (查询)");
	    printf("\033[%d;%dH",HIGH+6,COR+10);
		printf("2.Return the book(还书)");
	    printf("\033[%d;%dH",HIGH+9,COR+10);
	    printf("3.Borrow(借书)");
	    printf("\033[%d;%dH",HIGH+12,COR+10);
	    printf("4.Renew books(续借)");
        printf("\033[%d;%dH",HIGH+15,COR+10);
		printf("5.Home(主页)");
	    printf("\033[%d;%dH",HIGH+21,COR+10);
	    
	    i=in_selection(1,5);
	    if(i>0 && i<6)
         	return i;
	}

	return 0;
}                              //借书者界面
int admi()
{
	int i;
	clear1();
	while(1){
		drawbox();
	    printf("\033[%d;%dH",HIGH+3,COR+10);
	    printf("1.Search(查询)");
	    printf("\033[%d;%dH",HIGH+6,COR+10);
	    printf("2.Take out(取书)");
	    printf("\033[%d;%dH",HIGH+9,COR+10);
	    printf("3.Return(还书)");
	    printf("\033[%d;%dH",HIGH+12,COR+10);
	    printf("4.Update(更新整理)");
	    printf("\033[%d;%dH",HIGH+15,COR+10);
		printf("5.Home(主页)");
		printf("\033[%d;%dH",HIGH+16,COR+10);
	    i=in_selection(1,4);
	    if(i>0 && i<6)
         	return i;
	}
                                   //管理员选择界面界面
}

int login_check(char account[],char pass[],int n)   //account 和 pass 代表用户输入的账号和密码
{
	char a[N][N],b[N][N];                //a,b代表从文件中读出的账号和密码
	char pre[N],ch;                         //重新输入的密码
	int i=0,s=0,j;                               //n限制输入密码次数的变量
	FILE *fp;

	fp=fopen("admini.txt","rt+");                //只读当时打开文件
	if(fp==NULL ) 
	{
		printf("error8,press any key to exit\n");
		exit(1);                    
	}
    while(!feof(fp))
	{
		fscanf(fp,"%s %s ",a[i],b[i]);
		if( strcmp(account,a[i])==0 && strcmp(pass,b[i])==0 )   //核对账户和密码是否匹配
		{
 			printf("\033[%d;%dH",HIGH+15,COR+35);
			printf("Login success!");
			return 1;
		}	
	  i++;
	}
	if(feof(fp))
	{
		printf("\033[%d;%dH",HIGH+16,COR+31);
		if(n==0)
		{
			printf("登录失败");
			return 2;
		}
		printf("Without your information ");                  //如果文件里面账户和密码不符
	}

	printf("\033[%d;%dH",HIGH+5,COR+10);
    printf("1.Exit(退出该界面)  2.Again(重新输入密码)");
    

	while(s!=1 && s!=2){
                printf("\033[%d;%dH",HIGH+7,COR+12);
	    printf("Select:");
		scanf("%d",&s);
	}
	if(s==2)
	{
		clear1();                 //清屏以显示重新输密码界面
		drawbox();
		printf("\033[%d;%dH",HIGH+15,COR+10);
	    printf("Enter your password again");
	    printf("\033[%d;%dH",HIGH+5,COR+10);
	    printf("Password(6位数密码):");	

		for(j=0;j<6;j++)
		{
			scanf("%c",&pre[j]);
		//	printf("\033[K");
		}
	    pre[j]='\0';                  //再一次输入密码
		n--;
		login_check(account,pre,n);
	}

	
}                                   //输入密码并核对
int check()
{
	char account[N];
	char pass[N],ch;
	int i=0,j;
	drawbox();
	printf("\033[%d;%dH",HIGH+10,COR+10);
	printf("Enter your account number and password");
	printf("\033[%d;%dH",HIGH+3,COR+10);
	printf("Account(账号):");

	printf("\033[%d;%dH",HIGH+5,COR+10);
	printf("Password(6位数密码):");	

	printf("\033[%d;%dH",HIGH+3,COR+24);
	scanf("%s",account);
	
	getchar();
	printf("\033[%d;%dH",HIGH+5,COR+30);
	for(i=0;i<6;i++)
	{
		scanf("%c",&pass[i]);
	}
	pass[i]='\0';
                      //在界面输入账户及密码
	j=login_check(account,pass,3);

   return j;
}                         //登陆界面
int clear1()
{
	int i,j;
	for(i=0; i<100; i++)
		for(j=0; j<100; j++)
			printf("\033[%d;%dH ",i,j);
    printf("\033[1;1H");         //清屏完后光标返回到（1,1）位置
	return 0;
}                                //自定义清屏函数
BOOK *insert()
{
	BOOK *p;
	p=(BOOK *)malloc(sizeof(BOOK));
	if(p==NULL)
	{
		printf("error1:\n");
		exit(0);
	}

	clear1();
    printf("\033[%d;%dH",HIGH+3,COR+10);
	printf("input the price(价格)");
	scanf("%d",&p->price);
	if(p->price==0)
		return p;

	printf("\033[%d;%dH",HIGH+5,COR+10);
	printf("input num(图书编号):");
	scanf("%s",p->num);              //输入图书编号

	printf("\033[%d;%dH",HIGH+7,COR+10);
	printf("input bname(书名):");
	scanf("%s",p->bname);            //书名

	printf("\033[%d;%dH",HIGH+9,COR+10);
	printf("input wname(作者):");
	scanf("%s",p->wname);           //作者名

	printf("\033[%d;%dH",HIGH+11,COR+10);
	printf("Date of publication:");
	printf("\033[%d;%dH",HIGH+13,COR+10);
	printf("input year(年):");
	scanf("%d",&(p->publish.year) ); //年

	printf("\033[%d;%dH",HIGH+14,COR+10);
	printf("input month(月):");
	scanf("%d",&(p->publish.month) );//月

	printf("\033[%d;%dH",HIGH+15,COR+10);
        printf("input day(日):");
	scanf("%d",&(p->publish.day) );  //日

	printf("\033[%d;%dH",HIGH+16,COR+10);
	printf("input the number of books:");
	scanf("%d",&(p->number));        //编号

	return p;
}                                          //申请一个结点，录入图书信息
BOOK* instu()
{
	BOOK *p;
	p=(BOOK *)malloc(sizeof(BOOK));
	if(p==NULL)
	{
		printf("error1:\n");
		exit(0);
	}
	printf("Press 0 or 1(按0结束输入)");
	scanf("%d",&p->price);
	if(p->price==0)
		return p;
	printf("input numbers(学号):");
	scanf("%s",p->student.num);
	printf("input state(借书情况):");
	scanf("%d",&p->student.flag);
    p->student.day=0;
	return p;
}                                          //申请一个结点，录入学生信息
BOOK * creat(int i)
{
	BOOK *head,*pnew,*p;

	head=(BOOK* )malloc( sizeof(BOOK) );
	head->price=0;

	if(i==1)
		pnew=insert();
	if(i==2)
		pnew=instu();
	if(i==3)
		pnew=in_admini();
	if(pnew->price==0)
	{
		printf("error2\n");
		exit(0);
	}
	pnew->next=NULL;
	p=pnew;
	head->next=pnew;

	if(i==1)
		pnew=insert();
	if(i==2)
		pnew=instu();
	if(i==3)
		pnew=in_admini();
	while(pnew->price)
	{
		pnew->next=NULL;
		p->next=pnew;
		p=pnew;

		if(i==1)
			pnew=insert();
	    if(i==2)
			pnew=instu();
		if(i==3)
			pnew=in_admini();
	}

	free(pnew);
	p->next=NULL;
   return head;                      //创建一个带头结点的单链表用于录入信息
}
int print(BOOK* head, int i,int j,int m)   //m代表输出的输出的类型（例：学生）
{
	BOOK *p=head;
		if(p->next==NULL)
    	{
		    printf("error3\n");
		    return 1;
	    }
	//gotoxy(COR+i, HIGH+j);
        p=head->next;
     	while(p!=NULL)
	    {
			printf("\n\n");

			if(m==1)
			{
			printf("num(图书编号):%2s\t", p->num); 
			printf("bname(书名):%2s\t",   p->bname);
			printf("wname(作者名):%2s\n\n", p->wname);
	    	printf("prince(价格):%2d\t\t",  p->price);     
			printf("number(图书数量):%d\n\n",p->number);             //输出图书信息

			printf("Date of publication(出版日期):\t\t\t\t");
			printf("%2d.%2d.",p->publish.year,p->publish.month);    //正向输出
			printf("%2d",p->publish.day);
			printf("\n");
			}
			if(m==2)
			{
				printf("numbers(学号):%2s\t", p->student.num);
			    printf("(借书情况):%2d\t",p->student.flag);
				printf("day(借书天数):%d\t",p->student.day);
			}
			if(m==3)
			{
				printf("Account(账户):%2s\t",p->admini.account);
				printf("Password(密码):%2s\n",p->admini.password);
			
			}
			
	     	p=p->next;
	    }
	return 0;
}                                                   // 链表输出          
void free1(BOOK *head)
{
	BOOK *p=head->next;
	if(p==NULL)
	{
		printf("There is no data need to free\n");
		exit(0);
	}
	while(head!=NULL)
	{
		p=head->next;
		free(head);
		head=p;               //释放链表函数
	}
}                 
BOOK *in_admini()
{
	BOOK *p;
	p=(BOOK *)malloc( sizeof(BOOK) );
	if(p==NULL)
	{
		printf("error1:\n");
		exit(0);
	}
	printf("Press 0 or 1(按０结束输入)");
	scanf("%d",&p->price);
	if(p->price==0)
		return p;
	printf("Account(账号):");
	scanf("%s",p->admini.account);
	printf("Password(密码):");
	scanf("%s",p->admini.password);
	return p;
}
void save_inf(BOOK *head,int n)
{
	FILE *fp;
	BOOK *p;
	char filename[N];
	int i;
	printf("\033[%d;%dH",HIGH+8,COR+6);
	printf("Input the filenamet to save(文件名):");
	scanf("%s",filename);                             //输入文件名
	if(n==1)
		fp=fopen(filename,"wt");
	if(n==2)
		fp=fopen(filename,"at+");
	if(fp==NULL)
	{
		printf("error4,press any key to exit\n");
		exit(1);
	}
	printf("\033[%d;%dH",HIGH+10,COR+2);
	printf("Select storage type(1图书)(2学生)(3管理员):");
	while(1)
	{
		scanf("%d",&i);
		if(i==1 || i==2 || i==3)
			break;
		else
			printf("Error,input again:\n");
	}
	for(p=head->next; p!=NULL; p=p->next)
	{
		fputc('\n',fp);
		if(i==1)
		{
			fprintf(fp,"%s %s %d %s ",p->bname,p->wname,p->price,p->num);
		    fprintf(fp,"%d %d %d %d",p->publish.year,p->publish.month,p->publish.day,p->number);
		}
		if(i==2){
			fprintf(fp,"%s %d %d",p->student.num,p->student.flag,p->price);  //学号、状态、价格
		}
		if(i==3){
			fprintf(fp,"%s %s",p->admini.account,p->admini.password);
		}
	}
	printf("\033[%d;%dH",HIGH+15,COR+29);
	printf("Success!\n");
	fclose(fp);
}                                  //将链表的内容存到文件中
BOOK *read_inf(int i)
{
	BOOK * head,*pnew,*p;
	BOOK *tail;
	FILE *fp;
	char filename[N];

	//fflush(stdin);  //清缓存
	
    //gotoxy(COR+6,HIGH+5);
	//printf("Input the filenamet to read(文件名):");
   // scanf("%s",filename);
    if(i==1)
		fp=fopen("t.txt","rt+");
	if(i==2)
		fp=fopen("student.txt","rt+");
	if(i==3)
		fp=fopen("admini.txt","rt+");
    if(fp==NULL ) 
	{
		printf("error5,press any key to exit\n");
		exit(1);
	}                                  //用读写的方式打开一个文件

    head=(BOOK *)malloc(sizeof(BOOK));
	head->price=0;                    //头结点       
	
	if(!feof(fp))
	{
		pnew=(BOOK *)malloc(sizeof(BOOK));
		if(pnew==NULL){
		exit(1);
		}
		
		if(i==1)              //读图书
		{
			fgetc(fp);
			fscanf(fp,"%s %s %d %s ",pnew->bname,pnew->wname,&pnew->price,pnew->num);
	        fscanf(fp,"%d %d %d %d",&pnew->publish.year,&pnew->publish.month,&pnew->publish.day,&pnew->number);  //向链表第二个结点存入数据
		}
		if(i==2){
			fscanf(fp,"%s %d %d",pnew->student.num,&pnew->student.flag,&pnew->price);
		}                   //读学生信息
		if(i==3){
			fscanf(fp,"%s %s",pnew->admini.account,pnew->admini.password);
		}                   //读管理员信息
	    pnew->next=NULL;
	    tail=pnew;                   //建立双向关系
	    tail->next1=head;

	    p=pnew;
	    head->next=pnew;
	    head->next1=NULL;
		}
	 while(!feof(fp))
	 {
		 pnew=(BOOK *)malloc(sizeof(BOOK));
	     if(pnew==NULL)
		 {
			 printf("error6\n");
		     exit(0);
		 }
		 if(i==1)
		 {
			 fgetc(fp);
			 fscanf(fp,"%s %s %d %s ",pnew->bname,pnew->wname,&pnew->price,pnew->num);
	         fscanf(fp,"%d %d %d %d",&pnew->publish.year,&pnew->publish.month,&pnew->publish.day,&pnew->number);
		 }
		if(i==2){
			fscanf(fp,"%s %d %d",pnew->student.num,&pnew->student.flag,&pnew->price);
		}
		if(i==3){
			fscanf(fp,"%s %s",pnew->admini.account,pnew->admini.password);
		}
	   	 pnew->next=NULL;
		 tail=pnew;
		 tail->next1=p;

		 p->next=pnew;
		 p=pnew; 
	 }
	fclose(fp);
	p->next=NULL;
	return head;                //将文件内容读取到链表中返回链表头指针
}     
int func(char *a, char *b, int count1,int count2)
{
	int i,j,m;
	int max=0;
	int count[50]={0};
	if(a==NULL || b==NULL)
	{
		printf("error:\n");
		return -1;
	}
	for(i=0;i<=count1;i++)
	{
		m=i;
		for(j=0; j<=count2 && b[j] != a[m]; j++);     //传递参数时，短字符串及其字符长度在前面
		if(j<=count2)
		{
			for(; a[m]==b[j]; m++,j++)
				count[i]++;
		}
	}
	max=count[0];
	for(i=0; i<50; i++)
	{
		if( count[i]>max )
		{
			max=count[i];
		}
	}
	return max; 
}
void compare(char a[],int k)
{
	BOOK *head,*p,*pre=NULL;
	char num[N];
	int i=56,j=21;

	p=read_inf(1);
	p=p->next;
	while(p != NULL)
	{
		switch(k)
	   {
	       case 1:
			   strcpy(num,p->num);break;
		   case 2:
			   strcpy(num,p->wname);break;        
		   case 3:
			   strcpy(num,p->bname);break;
	   }
		if( strcmp(a,num) == 0)
		{
			head=(BOOK *)malloc(sizeof(BOOK));

	        head->next=NULL;
	        head->price=0;

			pre=insert1(p);                  //形成一条只带头结点和一个数据结点的链表        
			pre->next=NULL;
			head->next=pre;
			j+=8;
			print(head,i,j,1);             //将找到的一个结点信息输出

			free1(head);
		}

		p=p->next;
	}
	
}                               //将输入与文件中的信息进行比较
int query()
{
	int i=0;
	char num[N];
	BOOK  *head;
	
	   printf("\033[%d;%dH\033[35m\033[1m",HIGH+21,COR);
	   printf("1.Num(编号)\t");
	   printf("2.Author(作者)\t");
   	   printf("3.Name(书名)\t");
	   printf("4.Fuzzy.(模糊查）   ");
	   printf("5.All(显示所有图书)");
	   printf("\033[%d;%dH",HIGH+23,COR);
	   printf("\033[32m\033[1mSelect a query method( 选择查询方式 ):");
	   while(1)
	   {
		   printf("\033[%d;%dH",HIGH+23,COR+39);
		   scanf("%d",&i);
		   if(i>0 && i<6);
			   break;
	   }
	   if(i==4)
	   {
		   fuzzy_chinese();
		   return i;
	   }
	   if(i==5)
	   {
		   head=read_inf(1);
		   print(head,1,1,1);
		   free1(head);
		   return i;
	   }
	   printf("\033[%d;%dH\033[34m",HIGH+24,COR);
	   switch(i)
	   {
	       case 1:
			   printf("input the num:");break;
		   case 2:
			   printf("input the Author:");break;
		   case 3:
			   printf("input the name:");break;
	   }
	    scanf("%s",num);
			   compare(num,i);

return i;

}                         //多种查找
BOOK *insert1(BOOK *pre)
{
	BOOK *p;
	p=(BOOK *)malloc(sizeof(BOOK));
	if(p==NULL)
	{
		printf("error6:\n");
		exit(0);
	}
	strcpy(p->bname,pre->bname);
	strcpy(p->num,pre->num);
	strcpy(p->wname,pre->wname);

	p->price=pre->price;
	p->number=pre->number;
	p->publish.year=pre->publish.year;
	p->publish.month=pre->publish.month;
	p->publish.day=pre->publish.day;

	return p;
	
}                                          //将某一个结点图书数据保存

BOOK* ass(BOOK *p)                          /*保留值*/
{
	BOOK *t;
	t=(BOOK *)malloc(sizeof(BOOK));
	if(t==NULL)
	{
		printf("error\n");
		exit(0);
	}
	strcpy(t->bname,p->bname);                         //书名
	strcpy(t->wname,p->wname);                         //作者名
	strcpy(t->num,p->num);                             //编号
	t->number=p->number;                       //数量
	t->price=p->price;                         //价格

	t->publish.year=p->publish.year;          //出版日期
	t->publish.month=p->publish.month;
	t->publish.day=p->publish.day;
	
	return t;
}                                              
BOOK *PI(BOOK *head)
{
	BOOK *p=head, *pre=NULL, *t=NULL;
	BOOK *out=(head->next)->next;
	int flag=0;
	if(p->next==NULL)
	{
		printf("error\n");
		exit(0);
	}
	while(out!=NULL)       /*从第二个节点开始遍历*/
	{
		flag=0;
		p=NULL;
		t=ass(out);        /*保存一个结点的数据*/
		
		pre=out->next1;
		while(pre->price && (strcmp(pre->num,t->num) )>0 )
		{
			flag=1;
			pre=pre->next1;
		}
		if(flag==0)
		{
			out=out->next;
			free(t);
			t=NULL;
        }                          /*顺序由小到大*/
		else
		{
	    	  t->next=pre->next;
		      pre->next=t;

			  (t->next)->next1=t;
			  t->next1=pre;

		      p=out->next1;
			  if(out->next==NULL)
			  {
				p->next=NULL;
				free(out);         /*最后一个值out指向最后一个结点并且其值小于前面的数*/
			  }
			  else
			  {
				 p->next=out->next;
			     (out->next)->next1=p;

			     free(out);               //释放已被排序是结点值
			     out=p->next;
			  }	
		}
	}
	return head;
}
int admi_return()                           //管理员将书入库
{
	int i;
	char ch;
	BOOK *p=NULL,*head=NULL;

	head=(BOOK *)malloc(sizeof(BOOK));
    head->price=0;

	while(1)
	{
	    clear1();
	    drawbox();
	    printf("\033[%d;%dH",HIGH+18,COR+10);
		printf("Input basic information of books(出版信息)");

	    p=insert();      //把要还的书的信息输入到p中
		head->next=p;
		p->next=NULL;    //形成一条带一个结点的链表
		while(1)
		{
			clear1();
	        drawbox();
	        printf("\033[%d;%dH",HIGH+15,COR+10);
		    printf("Are you sure you want to deposit the book?");
			printf("\033[%d;%dH",HIGH+18,COR+10);
		    printf("你确定要存这本书吗?");
			printf("\033[%d;%dH",HIGH+10,COR+10);
			printf("Input y or n:");
		    scanf("%c",&ch);
			if(ch=='Y'||ch=='y'||ch=='n'||ch=='N')     //让用户输入是否还书只有输入y或n时才执行
				break;
		}
		if(ch=='Y'||ch=='y')
		{
			printf("\033[%d;%dH",HIGH+12,COR+10);
			printf("Success!<还书成功>");
			break;
		}
		else
		{
			free(p);                                //如果不还书则把p保存的图书信息释放
			p=NULL;

			clear1();
	                drawbox();
			printf("\033[%d;%dH",HIGH+12,COR+10);
			printf("1.Continue(重新还书)");
			printf("\033[%d;%dH",HIGH+12,COR+40);
			printf("2.Exit(退出)");                 //选择是否要重新还书

			printf("\033[%d;%dH",HIGH+14,COR+10);
			printf("select:");
			scanf("%d",&i);
			if(i==2)
				break;
		}
	}
	if(p!=NULL)
	{
         save_inf(head,2);
		 free(p);  
	}
                                  //还书完毕释放结点信息
	                              //管理员还书
}    
BOOK * fuzzy_chinese()
{
	int m=0,con1,con2;
	BOOK *head,*pnew=NULL,*r;
	BOOK *pre,*hh;
	char ch[N];

	head=(BOOK *)malloc(sizeof(BOOK));
	head->price=0;
	head->next=NULL;
	r=head;

	printf("Input(输入书籍部分名称:)");
	scanf("%s",ch);
	hh=read_inf(1);
	pre=hh->next;			//从文件读出所有图书


	while(pre != NULL)
    {
		   con1=strlen(ch);
		   con2=strlen(pre->bname);
		   m=func(ch,pre->bname,con1,con2);
		   if(m>=2)
		   {
			   pnew=ass(pre);        /*保存一个结点的数据*/
			   r->next=pnew;
			   r=pnew;
		   }

		   pre=pre->next;
	}
//lear1();
	r->next=NULL;
	if(r->price==0)
		printf("没有你要查的书");
	else
		print(head,1,1,1);
	free1(hh);
	
}
BOOK *del(BOOK * head,char a[])
{
	BOOK *p=head->next,*pre=head;
    while( p!=NULL && strcmp(p->bname,a))
	{
		pre=p;
		p=p->next;
	}
	if(p !=NULL )
	{
		pre->next=p->next;
		free(p);                 //删除带头结点的链表中的一个结点
	}

	else
	{
		printf("没有您所需书籍:");
	}
	return head;
}
int take_out()
{

	BOOK *head;
	char ch[N];
	head=read_inf(1);
	printf("输入书名:");
	scanf("%s",ch);
	head=del(head, ch);
	save_inf(head,1);
	
	free1(head);
	return 1;                   //管理员取书
}
int student_take(int i)
{
	char ch[N];
	BOOK *head=NULL,*p,*t,*pre;

	head=read_inf(2);
	p=head->next;

	printf("输入学号:");
	scanf("%s",ch);

	while(p != NULL && strcmp(ch,p->student.num))
	{
		pre=p;
		p=p->next;
	}
    if(p!=NULL)
	{
		if(i==1)                         //借书
		    t->student.flag=1;

		if(i==2)
	    	t->student.flag=0;           //还书  
		strcpy(t->student.num,p->student.num);

		t->next=p->next;
        pre->next=t;

		free(p);
	}

	save_inf(head,1);
	return 1;
}
int sort()
{
	BOOK *head;
	head=read_inf(1);
	PI(head);	
	save_inf(head,1);
	print(head,1,1,1);
	free1(head);
}
void fun_exe(int n)
{
	int i,j,k;
	switch(n)
	{
	    case 1:i=borrface();                          //借书者
			   switch(i)
			   {
			      case 1:query();break;              //借书者查询
				  case 2:take_out(2);break;        //还书
				  case 3:take_out(1);break;
				  case 4:break;
			   };
			   break;
	    case 2:j=check();                            //管理员选择功能
			   if(j==1){
				   k=admi();                        
			       switch(k)
				   {
			           case 1:query();break;        //管理员查询
				       case 2:take_out();break;     //管理员取书
				       case 3:admi_return();break;  //管理员还书
			           case 4:sort();break;
					   case 5:break;
				   }
			   };
                if(j==2)
				{}
			   break;                                //管理员
	}
}                            //执行选择功能

int subtra_ction()
{
	BOOK *head;

}
int main(void)
{
	int i,select=1;
	/*take_out();
	BOOK *head;
	head=creat(2);
    save_inf(head,1);
	free1(head);
	head=NULL;
	head=read_inf(2);
	printf("head,1,1,2");
    fuzzy_chinese();*/
	/*int i;
    int ch;
	int select=1;
	BOOK *head,*p;*/

	home();
	clear1();
	while(select)
	{
		i=face();
		clear1();
	    fun_exe(i);                    //执行功能
		sleep(2);                   //停顿2000秒
		printf("\033[%d;%dH",HIGH+25,COR+10);
		printf("Press 0 exit press any continue:");

		getchar();
	    clear1();
		drawbox();
		printf("\033[%d;%dH",HIGH+25,COR+10);
		printf("按0退出，按其他任意键继续:");
	    scanf("%d",&select);
	}
}
