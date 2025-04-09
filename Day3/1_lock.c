#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

#define THREAD_COUNT 10

void* thread_callback(void *arg){
    int *pcount = (int *)arg;
    
    int i = 0;
    while(i++ < 100000){
        (*pcount)++;
        //休眠1微秒
        usleep(1);
    }
}

int main(){
    pthread_t threadid[THREAD_COUNT] = {0};

    int i = 0;
    int count = 0;
    for (i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threadid[i], NULL, thread_callback, &count);
    }

    for (i = 0; i < 100; i++){
        printf("count: %d\n", count);
        //休眠1微秒
        sleep(1);
    }

}