#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
int value=0;
int main(){
	pid_t pid;
	pid=fork();
	if(pid<0){
		exit(0);
	}
	else if(pid==0){
	//	execlp("/usr/bin/top","top",NULL);
	for(;;)
	printf("%d child process\n",value++);
	}
	else{
		//wait(0);
	//	printf("child end\n");
		for(;;)
		printf("%d father process\n",value);
	}
	return 0;	
}
