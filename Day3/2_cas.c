#include<stdio.h>
#include<pthread.h>

#define THREAD_COUNT 10
#define ADD_COUNT 100000
#define COUT 500

int failed_count = 0;

int cas(int *ptr, int oldval, int newval){
    int result = 0;
    __asm__ volatile (
        "lock cmpxchgl %2, %1"
        :"=a" (result), "+m" (*ptr)
        :"r" (newval), "a" (oldval)
        :"memory"
    );
#if 0
    return result == oldval;
#else
    if(result == oldval){
        return 1;
    }
    else{
        failed_count++;
        printf("++Failed, oldval: %d, result: %d, failed_count: %d\n"
            , oldval, result, failed_count);
        return 0;
    }
}
#endif

int counter = 0;

void* theard_counter(void* arg){
    for(int i= 0; i < ADD_COUNT; i++){
        int oldval = 0;
        int newval = 0;
        do{
            oldval = counter;
            newval = oldval + 1;
        }while(!cas(&counter, oldval, newval));
    }
    return NULL;
}

int main(){
    pthread_t threads[THREAD_COUNT]={0};
    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_create(&threads[i], NULL, theard_counter, NULL);
    }
#if 0
    for(int i = 0; i < COUT; i++){
        printf("counter: %d\n", counter);
    }
#else
    for(int i = 0; i < THREAD_COUNT; i++){
        pthread_join(threads[i], NULL);
    }
    printf("counter: %d\n", counter);
#endif

    return 0;

}