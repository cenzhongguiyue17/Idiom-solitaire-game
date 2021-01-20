    #include <stdio.h> 
    #include <stdlib.h> 
    #include <unistd.h>
    #include <string.h>
    #include <sys/types.h> 
    #include <sys/socket.h> 
    #include <netinet/in.h> 
    #include <netdb.h>     
    #include<string.h>

    #define PORT 1234   
    #define MAXDATASIZE 100   

    char sendbuf[1024];
    char recvbuf[1024];
    char name[100];
    int fd;

    void pthread_recv(void* ptr)
{
    while(1)
    {
    if ((recv(fd,recvbuf,MAXDATASIZE,0)) == -1){ 
        printf("recv() error\n"); 
        exit(1); 
        } 
    printf("%s",recvbuf);
    memset(recvbuf,0,sizeof(recvbuf));
    }
}



    int main(int argc, char *argv[]) 
    { 
    int  numbytes;   
    char buf[MAXDATASIZE];   
    struct hostent *he;       
    struct sockaddr_in server;
    char str1[100];
	char str2[100];
	char str3[200];


    if (argc !=2) {         
	printf("Usage: %s <IP Address>\n",argv[0]); 
    exit(1); 
    } 


    if ((he=gethostbyname(argv[1]))==NULL){  
    printf("gethostbyname() error\n"); 
    exit(1); 
    } 

    if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){ 
    printf("socket() error\n"); 
    exit(1); 
    } 

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET; 
    server.sin_port = htons(PORT); 
    server.sin_addr = *((struct in_addr *)he->h_addr);  
    if(connect(fd, (struct sockaddr *)&server,sizeof(struct sockaddr))==-1){  
    printf("connect() error\n"); 
    exit(1); 
    } 

    printf("成功连接游戏\n");
    printf("——————游戏注意事项————————\n");
	printf("0.游戏必须先输入用户名进行注册\n");
	printf("1.输入成语不得更改\n");
	printf("2.接龙的字必须相同\n");
	printf("3.输入退出可以退出游戏\n");
	printf("4.游戏人数可以一至五个人\n");
    printf("5.胜利条件：您和其它玩家合力完成9次接龙\n");
	printf("--------------------------\n");
    printf("请输入用户名：");
	char str[]="已进入游戏室——";
    fgets(name,sizeof(name),stdin);
	strcat(str,name);
    send(fd,str,(strlen(str)),0);
	//fgets默认带\n，strlen()-1可以去掉
    strncpy(str1,name,strlen(name)-1);
	strcpy(str2,":");
	strcat(str1,str2);

    pthread_t tid;
    pthread_create(&tid,NULL,pthread_recv,NULL);

    while(1)
    {   
        
        memset(sendbuf,0,sizeof(sendbuf));
        fgets(sendbuf,sizeof(sendbuf),stdin);
        if(strcmp(sendbuf,"退出\n")==0){
            memset(sendbuf,0,sizeof(sendbuf));
            printf("您已退出游戏\n");
            send(fd,sendbuf,(strlen(sendbuf)),0);
            break;
        }
        
        strcpy(str3,str1);
		strcat(str3,sendbuf);
        send(fd,str3,(strlen(str3)),0);

    }

    close(fd);  
 }
