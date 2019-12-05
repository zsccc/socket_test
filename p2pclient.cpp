#include<stdio.h>
#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
#include<signal.h>
#include<stdlib.h>
#include<errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include<string>

using namespace std;
#define ERR_EXIT(m)\
	do\
{\
	perror(m);\
	exit(EXIT_FAILURE);\
}while(0)
#define svport 6666
void handler(int sig)
{
	cout<<"recv a sig= "<<sig <<endl;
	exit(EXIT_SUCCESS);
}
int main(){
	int sockclient;
	sockclient = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockclient <0){
		exit(0);
	}
	struct sockaddr_in clientaddr;
	memset(&clientaddr,0,sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port = htons(svport);
	clientaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	//inet_aton("127.0.0.1",&clientaddr.sin_addr);
	int on=1;
	if(setsockopt(sockclient,SOL_SOCKET,SO_REUSEADDR,(const char*)&on,sizeof(on)))
		exit(0);
	if(connect(sockclient,(struct sockaddr *)&clientaddr,sizeof(clientaddr))<0){
		exit(0);
	}
	pid_t pid;
	pid=fork();
	if(pid<0)
		exit(0);
	else if(pid==0)
	{	
		char recvbuf[1024];
		while(1){
			memset(recvbuf,0,sizeof(recvbuf));
			int ret = read(sockclient,recvbuf,sizeof(recvbuf));
			if(ret==-1)
				ERR_EXIT("read");
			else if(ret==0)
			{
				cout<<"peer close"<<endl;
				break;
			}
			else
			{
				fputs(recvbuf,stdout);
			}
		}
		close(sockclient);
		kill(getppid(),SIGUSR1);
	}
	else
	{	
		signal(SIGUSR1,handler);	
		char clientsendbuf[1024];
		while(fgets(clientsendbuf,sizeof(clientsendbuf),stdin)!=NULL)
		{
			write(sockclient,clientsendbuf,strlen(clientsendbuf));
			//cout<<clientrecvbuf<<endl;
			memset(clientsendbuf,0,sizeof(clientsendbuf));
		}
		close(sockclient);
	}
	//	close(sockclient);
	return 0;
}
