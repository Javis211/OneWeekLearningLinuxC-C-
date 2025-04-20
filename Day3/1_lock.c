#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

#define THREAD_COUNT 10

pthread_mutex_t mutex;
pthread_spinlock_t spinlock;

int voc(int *value, int add){
    int old;

    __asm__ volatile(
        "lock; xaddl %2, %1\n"
        :"=a" (old)
        :"m" (*value), "a" (add)
        :"cc", "memory"
    );

    return old;
}

void* thread_callback(void *arg){
    int *pcount = (int *)arg;
    
    int i = 0;
    while(i++ < 1000){
#if 0 
        (*pcount)++;
#elif 0
        pthread_mutex_lock(&mutex);
        (*pcount)++;
        pthread_mutex_unlock(&mutex);
#elif 0
        pthread_spin_lock(&spinlock);
        (*pcount)++;
        pthread_spin_unlock(&spinlock);
#else
        voc(pcount, 1);
#endif
        //休眠1微秒
        usleep(1);
    }
}

int main(){
    pthread_t threadid[THREAD_COUNT] = {0};
    pthread_mutex_init(&mutex, NULL);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

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

    pthread_mutex_destroy(&mutex);
    pthread_spin_destroy(&spinlock);

}