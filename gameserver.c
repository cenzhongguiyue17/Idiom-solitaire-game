    #include <stdio.h>          
    #include <string.h>         
    #include <unistd.h>          
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <netinet/in.h> 
    #include <arpa/inet.h>
    #include<pthread.h> 
    #include<stdlib.h>

    #define PORT 1234    
    #define BACKLOG 1 
    #define Max 5
    #define MAXSIZE 1024
	#define M 30806 //成语总数量
	#define VICTORY 10

/*定义全局变量*/
int fdt[Max]={0};
char mes[1024];
char cy[M][15];
int sendToAll; //当前消息是送给全体client，还是除当前client外的所有client
char playerCY[15][15]; //存储玩家输入的成语
int firstblank; //玩家输入的成语构成的数组第一个非空的行
int flag; //区分是系统提示信息还是用户输入的成语
/**/

void *pthread_service(void* sfd)
{ 
    
    int fd=*(int *)sfd;
    while(1)
    {
        int numbytes;
        int i,j;
        numbytes=recv(fd,mes,MAXSIZE,0);
        if(numbytes<=0){
            for(i=0;i<Max;i++){
                if(fd==fdt[i]){
                    fdt[i]=0;               
                }
            }
            printf("exit! fd=%d\n",fd);
            break;

        }else{
			printf("%s",mes);
			
			for(j=0;j<numbytes;j++){
				if(mes[j]==':'){
					flag=1;
					break;
				}
			}
			
			if(flag==0){
			 SendToClient(fd,mes,numbytes);
             bzero(mes,MAXSIZE);
			}else{
                SendToClient(fd,mes,numbytes);			
                getSubStr(mes);		
				judge(fd);//用户输入成语，需要用游戏规则对成语进行判断
				bzero(mes,MAXSIZE);
				flag=0;
			}
		}
        //printf("receive message from %d,size=%d\n",fd,numbytes);
       

    }
    close(fd);

}


/**/
int SendToClient(int fd,char* buf,int Size)
{
    int i;
	
	if(sendToAll==0){
      for(i=0;i<Max;i++){
        //printf("fdt[%d]=%d\n",i,fdt[i]);
        if((fdt[i]!=0)&&(fdt[i]!=fd)){
            send(fdt[i],buf,Size,0); 
        }
      }
	}else{
		
	  for(i=0;i<Max;i++){
        if((fdt[i]!=0)){
            send(fdt[i],buf,Size,0); 
        }
      }
	  sendToAll=0;
	}
	 return 0;
	}	
   




int getSubStr(char *p){
	char *p1;//截取字符串的指针
	int i=0;
	p1=strchr(p,':');
	while(*(p1+1)!='\n'){
		playerCY[firstblank][i]=*(p1+1);
		i++;
		p1++;
	}
	firstblank++;
	return 0;
	
}

int judge(int fd){
  char buf[512];
  int isFind=0; //玩家输入成语是否在成语库中
  int isEnter=0; //玩家是否已经输入过该成语
  int isSame=0; //相邻两成语首尾是否相同
  int k;
 
  //bzero(currentCY,15);
  for(k=0;k<M;k++){
  //检查成语是否在成语库中
	if(strncmp(playerCY[firstblank-1],cy[k],strlen(playerCY[firstblank-1]))==0){
		isFind=1;
		break;
	   }
					
	}
	if(isFind==0){
	  sendToAll=1;
	  strcpy(buf,"输入成语不存在，游戏失败,可以开始新一轮游戏或退出游戏\n\n");
	  SendToClient(fd,buf,strlen(buf));
	  bzero(buf,sizeof(buf));
	  memset(playerCY,0,sizeof(playerCY));
	  firstblank=0;
	}
	/**说明不是输入的第一个成语,需要检查是否其它玩家已经输入，以及当前成语的首字是否和
	   上一玩家输入成语的末字相同**/
	else{ 
	 if(firstblank>1){
	    int a;
	    for(a=0;a<firstblank-1;a++){	   
	     if(strcmp(playerCY[a],playerCY[firstblank-1])==0){
			strcpy(buf,"成语已经输入过了，游戏失败,可以开始新一轮游戏或退出游戏\n\n");
			sendToAll=1;
			isEnter=1;
			SendToClient(fd,buf,strlen(buf));
	        bzero(buf,sizeof(buf));
			memset(playerCY,0,sizeof(playerCY));
			firstblank=0;
			break;
		}	
	 }
	 
	 if(isEnter==0){
		 //判断两个成语是否首尾相同
		 if(playerCY[firstblank-1][0]==playerCY[firstblank-2][9]&&
		    playerCY[firstblank-1][1]==playerCY[firstblank-2][10]&&
			playerCY[firstblank-1][2]==playerCY[firstblank-2][11])   {
			
			isSame=1;
       			
		}
		
		 if(isSame==0){
			 
		   strcpy(buf,"相邻成语首尾不同，游戏失败，可以开始新一轮游戏或退出游戏\n\n");
		   sendToAll=1;
		   SendToClient(fd,buf,strlen(buf));
	       bzero(buf,sizeof(buf));
		   memset(playerCY,0,sizeof(playerCY));
		   firstblank=0;
		   
	     }else{
		    if(firstblank==VICTORY){
			 sendToAll=1;
			 strcpy(buf,"游戏胜利了，可以开始新一轮游戏或退出游戏\n\n");
			 SendToClient(fd,buf,strlen(buf));
	         bzero(buf,sizeof(buf));
			 memset(playerCY,0,sizeof(playerCY));
			 firstblank=0;
		 }
	 }
	 }
	 
	 	 
	}
	}
	return 0;
}

int  main()  
{ 

    int listenfd, connectfd;    
    struct sockaddr_in server; 
    struct sockaddr_in client;      
    int sin_size; 
    sin_size=sizeof(struct sockaddr_in); 
    int number=0;
    int fd;
	FILE *fp;

    if((fp=fopen("cy.txt","r"))==NULL)
	{
	  printf("cannot open file\n");
	  exit(1);
	}
	
	//将成语从文件库中读出，用char数组保存
	int j;
	for(j=0;j<M;j++)
	{
		fscanf(fp,"%s",cy[j]);
	}
	
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
    {   
        perror("Creating socket failed.");
        exit(1);
    }


    int opt = SO_REUSEADDR;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&server,sizeof(server));  


    server.sin_family=AF_INET; 
    server.sin_port=htons(PORT); 
    server.sin_addr.s_addr = htonl (INADDR_ANY); 


    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) { 
    perror("Bind error.");
    exit(1); 
    }   


    if(listen(listenfd,BACKLOG) == -1){  
    perror("listen() error\n"); 
    exit(1); 
    } 
    printf("等待玩家连接....\n");


    while(1)
    {

        if ((fd = accept(listenfd,(struct sockaddr *)&client,&sin_size))==-1) {
        perror("accept() error\n"); 
        exit(1); 

        }

        if(number>=Max){
            printf("no more client is allowed\n");
            close(fd);
        }

        int i;
        for(i=0;i<Max;i++){
            if(fdt[i]==0){
                fdt[i]=fd;
                break;
            }

        }



        pthread_t tid;
        pthread_create(&tid,NULL,(void*)pthread_service,&fd);

        number=number+1;


    }


    close(listenfd);            
    }
