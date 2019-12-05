#include <iostream>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
using namespace std;

void *run(void *ptr){
	int value=*(int *)ptr;
        for(int i=0; i<3; i++) {

          sleep(1);
                cout<<"value "<<value<<endl;
        }
        return 0;
}
int main(){
        int ret=0;
        int value=10;
        pthread_t id;
        pthread_attr_t attr;
	sched_param param;
        //初始化
        pthread_attr_init(&attr);
        //设置相关属性
        pthread_attr_setscope (&attr,PTHREAD_SCOPE_PROCESS);
	//获取线程优先级参数
        pthread_attr_getschedparam(&attr,&param);
	 //设置优先级
        param.sched_priority=10;
	pthread_attr_setschedparam(&attr,&param);
        ret=pthread_create(&id,&attr,run,&value);
        if(ret) {
                cout<<"create thread failed "<<endl;
                return 0;
        }
        pthread_join(id,NULL);
        return 0;
}

