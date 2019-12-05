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
struct packet{
	int len;
	char buf[1024];
};
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

void serv_con(int conn)
{

	struct packet recevbuf;
	while(1){
		memset(&recevbuf,0,sizeof(recevbuf));
		int ret=readn(conn,&recevbuf.len,4);
		//		cout<<recevbuf<<endl;
		if(ret==-1)
		{
			ERR_EXIT("read");	
		}
		else if(ret<4)
		{
			cout<<"client close"<<endl;
			break;
		}
		int n = ntohl(recevbuf.len);
		ret = readn(conn,recevbuf.buf,n);
		if(ret==-1)
		{
			ERR_EXIT("read");	
		}
		else if(ret<n)
		{
			cout<<"client close"<<endl;
			break;
		}
		fputs(recevbuf.buf,stdout);
		writen(conn,&recevbuf,4+n);
	}
}
int main(){
	int listenfd;
	listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(listenfd <0){
		exit(0);
	}
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(svport);
	cout<<ntohs(servaddr.sin_port)<<endl;
	servaddr.sin_addr.s_addr= htonl(INADDR_ANY);//本机的任意地址
	//servaddr.sin_addr.s_addr= inet_addr("127.0.0.1");
	//inet_aton("127.0.0.1",&servaddr.sin_addr);	
	bool portrestart = 1;
	int on=1;

	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)
		//		if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const bool*)&portrestart,sizeof(portrestart))<0)
	{
		cout<<"zsc1111"<<endl;
		ERR_EXIT("setsockopt");
		cout<<"xxx"<<endl;
	}
	cout<<"zsc"<<endl;
	int blfd=bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if(blfd<0)
		exit(0);
	int lfd = listen(listenfd,SOMAXCONN);
	if(lfd<0)
		exit(0);
	struct sockaddr_in peeraddr;
	socklen_t peerlen= sizeof(peeraddr);
	int conn ;
	while(1){

		if ((conn=accept(listenfd,(struct sockaddr *)&peeraddr,&peerlen))<0)
			exit(0);
		cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<"port="<<ntohs(peeraddr.sin_port)<<endl;
		pid_t pid ;
		pid = fork();
		if(pid<0)
			ERR_EXIT("fork");
		if(pid==0)
		{
			close(listenfd);
			serv_con(conn);
			close(conn);
			exit(EXIT_SUCCESS);
		}	
		else
		{
			close(conn);
		}
	}
	close(listenfd);
	return 0;
}
