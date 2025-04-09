## 【Linux入门环境编程】多线程并发锁+数据库MySQL

### 1 并发下的技术方案——多线程并发锁项目

#### 项目背景

- 什么是多线程？
- 什么是并发执行？

1. ##### 线程状态 

线程是cpu任务调度的最小执行单位，每个线程拥有自己独立的程序计数器、虚拟机栈、本地方法栈。 

**线程状态：创建、就绪、运行、阻塞、死亡**

![img](https://cdn.nlark.com/yuque/0/2024/png/22811459/1714393304366-061ec09e-e96b-4fff-85b6-4b5c503f4cad.png?x-oss-process=image%2Fwatermark%2Ctype_d3F5LW1pY3JvaGVp%2Csize_19%2Ctext_5Zu-54G16K--5aCC%2Ccolor_FFFFFF%2Cshadow_50%2Ct_80%2Cg_se%2Cx_10%2Cy_10)

2. **C语言中线程的使用**

   ​	在 C 语言里，pthread_t 是用来表示线程标识符的数据类型。借助 POSIX 线程库（pthread）能够创建、管理和同步线程。

- ######  创建线程

  创建线程要用到 pthread_create 函数，其原型如下：

  ```
  int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                  void *(*start_routine) (void *), void *arg);
  ```

  - thread：指向 pthread_t 类型的指针，用于存储新创建线程的标识符。

  - attr：指向线程属性的指针，通常设为 NULL 来使用默认属性。

  - start_routine：线程启动后要执行的函数指针，此函数的返回类型为 void *，参数类型为 void *。

  - arg：传递给 start_routine 函数的参数。

- ###### 引用线程

  创建线程之后，你可以使用 pthread_join 函数来等待线程结束，并且获取其返回值。pthread_join 函数的原型如下：

  ```
  int pthread_join(pthread_t thread, void **retval);
  ```

  - thread：要等待的线程的标识符。

  - retval：指向 void * 类型的指针，用于存储线程的返回值。

- ###### 线程锁

​	为了避免多个线程同时访问共享资源而引发竞态条件，你可以使用线程锁。POSIX 线程库提供了互斥锁（mutex），常用的函数如下：

 - pthread_mutex_init：初始化互斥锁:

```
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
```

- pthread_mutex_lock：对互斥锁加锁。

```
int pthread_mutex_lock(pthread_mutex_t *mutex);
```

- pthread_mutex_unlock：对互斥锁解锁。

```
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

- pthread_mutex_destroy：销毁互斥锁。

```
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

执行下面这段代码，启动 10 个线程并行执行累加操作，观察是否会出现线程抢占资源（即线程相互竞争使用共享资源）的情况。

注：

- usleep是休眠1微秒；

- pthread_create传入的是地址；

- 编译时需要链接`pthread`库，使用如下命令：

  ```sh
  gcc 1_lock.c -o 1_lock -lpthread
  ```

```
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
```

##### 问题：

- 为什么我的输出是一次全部输出的，是因为虚拟机开的内核数不够多吗？
- 



