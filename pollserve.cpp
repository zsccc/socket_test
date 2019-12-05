#include<stdio.h>
#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<errno.h>
#include<signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include<string>
#include<poll.h>
#include<sys/wait.h>
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

ssize_t recv_peek(int sockfd, void *buf, size_t len){
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
		//		cout<<recevbuf<<endl;
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
		//		wait(NULL);
		while(waitpid(-1,NULL,WNOHANG)>0);
	}
}

int main(){
	int listenfd;
	signal(SIGCHLD,SIG_IGN);
	signal(SIGCHLD,handle_sigchld);
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
		ERR_EXIT("bind");
	int lfd = listen(listenfd,SOMAXCONN);
	if(lfd<0)
		ERR_EXIT("listenfd");
	struct sockaddr_in peeraddr;
	socklen_t peerlen ;
	int conn ;
	struct pollfd client[2048];
	int maxi=0;
	for(int i=0;i<2048;i++)
		client[i].fd=-1;
	int nready;
	client[0].fd=listenfd;
	client[0].events=POLLIN;
	while(1)
	{
//		for(int k=0;k<3;k++)
//		{
//			cout<<"k="<<k<<"revents="<<client[k].revents<<endl;
//		}
		nready=poll(client,maxi+1,-1);
		if(nready==-1)
		{
			if(errno==EINTR)
				continue;
			ERR_EXIT("select");
		}
		else if (nready==0)
			continue;
		if(client[0].revents&POLLIN)
		{
			peerlen= sizeof(peeraddr);
			conn=accept(listenfd,(struct sockaddr *)&peeraddr,&peerlen);
			cout<<"conn ="<<conn<<endl;
			if(conn==-1)
				ERR_EXIT("accept");
			bool mark= false;
			for(int i=0;i<2048;i++)
			{
				if(client[i].fd<0)
				{
					mark=true;
					client[i].fd=conn;
					client[i].events=POLLIN;
					if(i>maxi)
						maxi=i;
					break;
				}

			}
			if(!mark)
			{
				fprintf(stderr,"too many clients\n");
				exit(EXIT_FAILURE);
			}
			cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<"port="<<ntohs(peeraddr.sin_port)<<endl;
			if(--nready<=0)
				continue;
		}
		for(int i=1;i<=maxi;i++)
		{
			conn=client[i].fd;
			if(conn==-1)
				continue;
			if(client[i].revents&POLLIN)
			{
				char recevbuf[1024]={0};
//				cout<<"i="<<i<<"revents="<<client[i].revents<<endl;
				int ret=readline(conn,&recevbuf,1024);
				//              cout<<recevbuf<<endl;
				if(ret==-1)
				{
					ERR_EXIT("readline");
				}
				if(ret==0)
				{
					cout<<"client close"<<endl;
					client[i].fd=-1;
					close(conn);
				}
				fputs(recevbuf,stdout);
				writen(conn,recevbuf,strlen(recevbuf));
				if(--nready<0)
				 	break;

			}
		}

	}
	return 0;
}
