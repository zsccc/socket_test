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
	if ((conn=accept(listenfd,(struct sockaddr *)&peeraddr,&peerlen))<0)
		exit(0);
	cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<"port="<<ntohs(peeraddr.sin_port)<<endl;
	char recevbuf[1024];
	while(1){
		memset(recevbuf,0,sizeof(recevbuf));
		int ret=read(conn,recevbuf,sizeof(recevbuf));
		//		cout<<recevbuf<<endl;
		fputs(recevbuf,stdout);
		write(conn,recevbuf,ret);
	}	
	close(conn);
	close(listenfd);
	return 0;
}
