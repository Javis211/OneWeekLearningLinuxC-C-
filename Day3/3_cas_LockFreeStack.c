#include<stdio.h>
#include<pthread.h>



int cas(int *ptr, int oldval, int newval){
    int result = 0;
    __asm__ volatile(
        "lock cmpxchg %2, %1"
        :"=a" (result), "+m" (*ptr)
        :"r" (newval), "+a" (oldval)
        :"memory"
    );
    return result == oldval;
}

int counter = 0;

void* theard_counter(void* arg){
    for(int i= 0; i < 1000; i++){
        int oldval;
        int newval;
        cas(&counter, oldval, newval);
    }
}