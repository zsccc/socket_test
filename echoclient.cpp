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
	struct packet  clientsendbuf;
	struct packet  clientrecvbuf;
	memset(&clientsendbuf,0,sizeof(clientsendbuf));
	memset(&clientrecvbuf,0,sizeof(clientrecvbuf));
	while(fgets(clientsendbuf.buf,sizeof(clientsendbuf.buf),stdin)!=NULL)
	{
		int n=strlen(clientsendbuf.buf);
		clientsendbuf.len=htonl(n);
		writen(sockclient,&clientsendbuf,4+n);
		int ret=readn(sockclient,&clientrecvbuf.len,4);
                //              cout<<recevbuf<<endl;
                if(ret==-1)
                {
                        ERR_EXIT("read");
                }
                else if(ret<4)
                {
                        cout<<"client close"<<endl;
                        break;
                }
                 int m = ntohl(clientrecvbuf.len);
                ret = readn(sockclient,clientrecvbuf.buf,m);
                if(ret==-1)
                {
                        ERR_EXIT("read");
                }
                else if(ret<m)
                {
                        cout<<"client close"<<endl;
                        break;
                }

		//cout<<clientrecvbuf<<endl;
		fputs(clientrecvbuf.buf,stdout);
		memset(&clientsendbuf,0,sizeof(clientsendbuf));
		memset(&clientrecvbuf,0,sizeof(clientrecvbuf));
	}
	close(sockclient);
	return 0;
}
