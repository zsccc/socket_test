#include<stdio.h>
#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
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

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft=count;
	ssize_t nread;//接收到的字节数;
	char * bufp=(char *)buf;
	while(nleft>0)
	{
		if((nread=read(fd,bufp,nleft))<0)
		{
			if(errno==EINTR)
				continue;
			return -1;
		}
		else if(nread==0)
		{
			return count-nleft;
		}
		else
		{
			bufp+=nread;
			nleft-=nread;
		}
	}
	return count;
}
ssize_t writen(int fd, const void *buf, size_t count)
{

	size_t nleft=count;
	ssize_t nwritten;
	char * bufp=(char *)buf;
	while(nleft>0)
	{
		if((nwritten=write(fd,bufp,nleft))<0)
		{
			if(errno==EINTR)
				continue;
			return -1;
		}
		else if(nwritten==0)
		{
			continue;
		}
		else
		{
			bufp+=nwritten;
			nleft-=nwritten;
		}
	}
	return count;
}
size_t recv_peek(int sockfd, void *buf, size_t len){
	while(1)
	{
		int ret=recv(sockfd,buf,len,MSG_PEEK);
		if(ret==-1&&errno==EINTR)
			continue;
		return ret;
	}
}

ssize_t readline(int sockfd, void *buf, size_t maxline){
	int ret;
	int nread;
	char *bufp=(char *)buf;
	int nleft=maxline;
	while(1){
		ret=recv_peek(sockfd,bufp,nleft);
		if(ret<0)
			return ret;
		else if(ret==0)
			return ret;
		nread=ret;
		for(int i=0;i<nread;i++){
			if(bufp[i]=='\n'){
				ret=readn(sockfd,bufp,i+1);
				if(ret!=i+1)
					exit(EXIT_FAILURE);
				return ret;
			}    
		}
		if(nread>nleft)
			exit(EXIT_FAILURE);
		nleft-=nread;
		ret=readn(sockfd,bufp,nread);
		if(ret!=nread)
			exit(EXIT_FAILURE);
		bufp+=nread;
	}
	return -1; 

}

void echo_cli(int sockclient)
{
	//	char  clientsendbuf[1024]={0};
	//	char  clientrecvbuf[1024]={0};
	//	while(fgets(clientsendbuf,sizeof(clientsendbuf),stdin)!=NULL)
	//	{
	//		writen(sockclient,clientsendbuf,strlen(clientsendbuf));
	//		int ret=readline(sockclient,clientrecvbuf,sizeof(clientrecvbuf));
	//		//              cout<<recevbuf<<endl;
	//		if(ret==-1)
	//		{
	//			ERR_EXIT("readline");
	//		}
	//		else if(ret==0)
	//		{
	//			cout<<"client close"<<endl;
	//			break;
	//		}
	//		//cout<<clientrecvbuf<<endl;
	//		fputs(clientrecvbuf,stdout);
	//		memset(clientsendbuf,0,sizeof(clientsendbuf));
	//		memset(clientrecvbuf,0,sizeof(clientrecvbuf));
	//	}
	//	close(sockclient);

	char  clientsendbuf[1024]={0};
	char  clientrecvbuf[1024]={0};
	fd_set rset;
	FD_ZERO(&rset);
	int nready;
	int maxfd;
	int fd_stdin=fileno(stdin);//fileno获得stdin的文件描述符
	if(fd_stdin>sockclient)
		maxfd=fd_stdin;
	else
		maxfd=sockclient;
	int stdineof=0;
	while(1)
	{
		if(!stdineof)
			FD_SET(fd_stdin,&rset);
		FD_SET(sockclient,&rset);
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if(nready==-1)
			ERR_EXIT("select");	
		if(nready==0)//超时
			continue;
		if(FD_ISSET(sockclient,&rset))
		{
			int ret=readline(sockclient,clientrecvbuf,sizeof(clientrecvbuf));
			//              cout<<recevbuf<<endl;
			if(ret==-1)
			{
				ERR_EXIT("readline");
			}
			else if(ret==0)
			{
				cout<<"server close"<<endl;
				break;
			}
			//cout<<clientrecvbuf<<endl;
			fputs(clientrecvbuf,stdout);
			memset(clientrecvbuf,0,sizeof(clientrecvbuf));

		}
		else if(FD_ISSET(fd_stdin,&rset))
		{
			if(fgets(clientsendbuf,sizeof(clientsendbuf),stdin)==NULL)
			{
				stdineof=1;
				shutdown(sockclient,SHUT_WR);	
			}
			else
			{
				writen(sockclient,clientsendbuf,strlen(clientsendbuf));
				memset(clientsendbuf,0,sizeof(clientsendbuf));
			}
		}
	}
	close(sockclient);
}


void handle_sigpipe(int sig)
{
	cout<<"sig ="<<sig<<endl;
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
	struct sockaddr_in localaddr;
	socklen_t addrlen= sizeof(localaddr);
	if(getsockname(sockclient,(struct sockaddr*)&localaddr,&addrlen))
	{
		ERR_EXIT("getsockname");
	}
	cout<<"ip="<<inet_ntoa(localaddr.sin_addr)<<"port="<<ntohs(localaddr.sin_port)<<endl;
	echo_cli(sockclient);
	return 0;
}
