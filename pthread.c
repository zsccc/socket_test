#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
unsigned long long value;
int thread_exit=0;
void * adder(void * param);
int main(){
	int i;
	pthread_t tid_adder;
	pthread_create(&tid_adder,NULL,adder,NULL);
	for(;;){
		printf("value is %llu\n",value);
		if(value>100000000)
			thread_exit=1;
		sleep(1);
	}
	return 0;
}
void *adder(void *param){
	while(!thread_exit)
		value++;
	return NULL;
}
