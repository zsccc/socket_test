#include<stdio.h>
#include<sys/epoll.h>
#include<sys/wait.h>
#include<errno.h>
#include<vector>
#include<algorithm>
#include<fcntl.h>
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

typedef vector<struct epoll_event> EventList;
#define svport 6666
void activate_nonblock(int fd)
{
	int ret;
	int flags=fcntl(fd,F_GETFL);
	if(flags==-1)
		ERR_EXIT("fcntl");
	flags|=O_NONBLOCK;
	ret=fcntl(fd,F_SETFL,flags);
	if(ret==-1)
		ERR_EXIT("fcntl");
}
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

ssize_t recv_peek(int sockfd,void *buf,size_t len)
{
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

void serv_con(int conn)
{

	char  recevbuf[1024];
	while(1){
		memset(recevbuf,0,sizeof(recevbuf));
		int ret=readline(conn,&recevbuf,1024);
		//              cout<<recevbuf<<endl;
		if(ret==-1)
		{
			ERR_EXIT("readline");
		}
		if(ret==0)
		{
			cout<<"client close"<<endl;
			break;
		}
		fputs(recevbuf,stdout);
		writen(conn,recevbuf,strlen(recevbuf));
	}
}
void handle_sigchld(int sig)
{
	if(sig==SIGCHLD)
	{
		cout<<"xxxxzzzzzz"<<endl;
		//              wait(NULL);
		while(waitpid(-1,NULL,WNOHANG)>0);
	}
}

void handle_sigpipe(int sig)
{
	cout<<"sig ="<<sig<<endl;
}
int main(){
	int listenfd;
	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,handle_sigchld);
	signal(SIGPIPE,handle_sigpipe);
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
		//              if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const bool*)&portrestart,sizeof(portrestart))<0)
	{
		ERR_EXIT("setsockopt");
	}
	int blfd=bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if(blfd<0)
		ERR_EXIT("bind");
	int lfd = listen(listenfd,SOMAXCONN);
	if(lfd<0)
		ERR_EXIT("listenfd");
	vector<int> clients;
	int epollfd;
	epollfd=epoll_create1(EPOLL_CLOEXEC);
	struct epoll_event event;
	event.data.fd=listenfd;
	event.events=EPOLLIN|EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&event);
	EventList events(16);
	struct sockaddr_in peeraddr;
	socklen_t peerlen ;
	int conn ;
	int nready;
	while(1)
	{
		nready=epoll_wait(epollfd,&*events.begin(),static_cast<int>(events.size()),-1);

		if(nready==-1)
		{
			if(errno==EINTR)
				continue;
			ERR_EXIT("epoll_wait");
		}
		else if (nready==0)
			continue;
		if((size_t)nready==events.size())
			events.resize(events.size()*2);
		for(int i=0;i<nready;i++)
		{
			if(events[i].data.fd==listenfd)
			{
				peerlen=sizeof(peeraddr);
				conn=accept(listenfd,(struct sockaddr *)&peeraddr,&peerlen);
				if(conn==-1)
					ERR_EXIT("accept");
				cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<"port="<<ntohs(peeraddr.sin_port)<<endl;
				clients.push_back(conn);
				activate_nonblock(conn);
				event.data.fd=conn;
				event.events=EPOLLIN|EPOLLET;
				epoll_ctl(epollfd,EPOLL_CTL_ADD,conn,&event);
			}
			else if(events[i].events&EPOLLIN)
			{
				conn=events[i].data.fd;
				if(conn<0)
					continue;
				char recvbuf[1024]={0};
				int ret=readline(conn,recvbuf,1024);
				if(ret==-1)
					ERR_EXIT("readline");
				if(ret==0)
				{
					cout<<"client close"<<endl;
					close(conn);
					event=events[i];
					epoll_ctl(epollfd,EPOLL_CTL_DEL,conn,&event);
					clients.erase(remove(clients.begin(),clients.end(),conn),clients.end());
				}
				fputs(recvbuf,stdout);
				writen(conn,recvbuf,strlen(recvbuf));
			}
		}
		
	}
	return 0;
}

