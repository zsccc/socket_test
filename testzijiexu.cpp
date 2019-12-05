#include<stdio.h>
#include<arpa/inet.h>
int main(){
	unsigned int x=0x12345678;
	unsigned char *p= (unsigned char *)&x;
	printf("%0x %0x %0x %0x\n",p[0],p[1],p[2],p[3]);
	x=htonl(x);
	printf("%0x %0x %0x %0x\n",p[0],p[1],p[2],p[3]);
	unsigned long addr = inet_addr("192.168.1.1");
	printf("addr = %u \n",ntohl(addr));
	struct in_addr ipaddr;
	ipaddr.s_addr= addr;
	printf("%s\n",inet_ntoa(ipaddr));
	return 0;
}
