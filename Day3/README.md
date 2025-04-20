## 【Linux入门环境编程】多线程并发锁

### 1 并发锁方案

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

  ```
  gcc 1_lock.c -o 1_lock -lpthread
  ```

- `-l`是gcc的一个选项，其用途是指定要链接的库。当你在命令里使用-lpthread时，gcc会尝试去链接名为pthread的库。gcc实际上会去寻找名为libpthread.a（静态库）或者libpthread.so（动态库）的文件

- `-L`选项指定的路径：在gcc命令里，你可以使用-L选项来指定额外的库文件查找路径。

- 

  

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

- 在 C 语言中，当你使用 {0} 对数组进行初始化时，如果初始化列表中的元素数量少于数组的元素数量，那么剩余的元素会被自动初始化为 0
- 通过检查 threadid[i] 是否为 0，就能判断对应的线程是否已成功创建，进而决定是否需要调用 pthread_join 等待线程结束

输出结果：count并没有加到1000000，因为count是一个临界资源，

![并发锁方案](Z:\0voice\Linux入门环境编程\3. 并发锁方案\并发锁方案.png)

#### 多线程并发锁的解决方案

##### 1 互斥锁

互斥锁（Mutex）是一种用于多线程或多进程编程中的同步原语，其主要作用是保证在同一时刻，只有一个线程或进程能够访问共享资源，以此避免多个线程或进程同时操作共享资源而引发的数据不一致、竞争条件等问题。

1. **定义互斥锁**：使用 `pthread_mutex_t`定义一个互斥锁mutex。
2. **初始化互斥锁**：在 `main` 函数中，使用 `pthread_mutex_init(&mutex, NULL)` 函数初始化互斥锁。
3. **线程函数**：`thread_function` 是线程执行的函数，在函数内部，使用 `pthread_mutex_lock(&mutex)` 锁定互斥锁，访问共享资源，然后使用 `pthread_mutex_unlock(&mutex)` 解锁互斥锁。
4. **等待线程结束**：使用 `pthread_join(thread)` 函数等待两个线程结束。
5. **销毁互斥锁**：使用 `pthread_mutex_destroy(mutex)` 函数销毁互斥锁。

##### 2 自旋锁

自旋锁（pthread_spin_t）是一种用于线程同步的机制，和互斥锁（pthread_mutex_t）类似，不过工作方式有所不同。当一个线程尝试获取一个已经被其他线程持有的自旋锁时，它不会像互斥锁那样进入阻塞状态，而是会不断地循环检查锁是否可用，这种不断循环等待的行为就是 “自旋”。

	1. **初始化自旋锁**：`pthread_spin_init(pthread_spinlock_t *lock, int pshared)`

- `lock`：指向自旋锁对象的指针。
- `pshared`：指定自旋锁的共享属性，`PTHREAD_PROCESS_SHARED` 表示可以在多个进程间共享，`PTHREAD_PROCESS_PRIVATE` 表示只能在同一个进程的多个线程间共享。

2. **锁定自旋锁**`pthread_spin_lock(pthread_spinlock_t *lock)`

3. **尝试锁定自旋锁**`pthread_spin_trylock(pthread_spinlock_t *lock)`

- **返回值**：如果锁可用，锁定并返回 `0`；如果锁已经被其他线程持有，返回 `EBUSY`。

4. **解锁自旋锁**`pthread_spin_unlock(pthread_spinlock_t *lock);`

5. **销毁自旋锁**`pthread_spin_destroy(pthread_spinlock_t *lock);`

##### 3 自旋锁和互斥锁的区别

###### 实现区别：

- **自旋锁**：当一个线程尝试获取已经被其他线程持有的自旋锁时，它不会进入阻塞状态，而是会持续不断地检查锁是否可用，也就是在原地 “自旋” 等待。这种方式会持续占用 CPU 资源，不断进行忙等待。
- **互斥锁**：若线程尝试获取已被其他线程持有的互斥锁，该线程会被操作系统挂起，进入阻塞状态，释放 CPU 资源。直到持有锁的线程释放锁后，操作系统会唤醒等待该锁的线程，让其有机会获取锁。

###### 适用场景：

- **自旋锁**：由于自旋过程会一直占用 CPU，在**锁的持有时间较短**的情况下，自旋锁的性能表现较好。因为避免了线程上下文切换带来的开销，线程可以快速获取到锁并继续执行。但如果锁的持有时间较长，会造成大量的 CPU 资源浪费，降低系统整体性能。例如，**赋值操作**等
- **互斥锁**：线程阻塞和唤醒操作会涉及到操作系统的调度，存在一定的上下文切换开销。因此，在锁的持有时间较短时，频繁的上下文切换会使互斥锁的性能不如自旋锁。但在**锁的持有时间较长**的场景下，互斥锁能让线程在等待时释放 CPU 资源，供其他线程使用，提高了系统的整体资源利用率。例如，**线程安全的红黑树**等。

##### 4 原子操作——CPU多条指令指令合并为单条指令

 原子操作指的是在执行过程中不可被中断的操作，即该操作一旦开始，就会一直执行到结束，期间不会被其他线程或进程打断。在多线程或多进程环境里，原子操作能够确保对共享资源的访问是线程安全的，避免出现数据竞争和不一致的问题。

使用 `__asm__ volatile` 关键字嵌入汇编代码，`volatile` 关键字可以告诉编译器不要对汇编代码进行优化，使用内联汇编执行 xaddl 指令：

```
int inc(int *value, int add) {
	int old;
    __asm__ volatile(
        "lock; xaddl %2, %1;"
        : "=a" (old)
        : "m" (*value), "a"(add)
        : "cc", "memory"
    );
    return old;
}
```

- **lock 前缀**：lock 前缀能够确保在执行 xaddl 指令期间，总线被锁定，这样就避免了其他处理器同时访问该内存地址，从而实现原子操作。
- **操作数**：
  - `%2` 代表 `add`，这是要加到内存地址上的值。
  - `%1` 代表 `*value`，也就是要进行加法操作的内存地址。
- **输出约束**：`"=a" (old)` 表示将 `eax` 寄存器的旧值存储到变量 `old` 中。
- **输入约束**：
  - `"m" (*value)` 表明 `*value` 是一个内存操作数。
  - `"a"(add)` 表示将 `add` 的值放入 `eax` 寄存器。
  - 记得输入操作没有读写操作符。
- **破坏列表**：
  - `"cc"` 意味着指令会修改标志寄存器。
  - `"memory"` 表示指令会访问内存。

##### 完整代码

```
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
```

##### 5 作业：了解CAS并实现CAS

CAS（Compare-And-Swap）即比较并交换，是一种实现并发算法时常用到的技术，用于实现多线程环境下的原子操作。CAS 操作包含三个操作数 —— 内存位置（V）、预期原值（A）和新值（B）。执行 CAS 操作时，会先比较内存位置 V 中的值是否与预期原值 A 相等，如果相等，则将内存位置 V 中的值更新为新值 B；如果不相等，则不做任何操作。整个操作是原子性的，在执行过程中不会被其他线程打断。

- x86下CAS的实现：

```
int x86_cas(int* ptr, int oldval, int newval) {
    int result;
    __asm__ volatile (
        "lock cmpxchgl %2, %1"
        : "=a" (result), "+m" (*ptr)
        : "r" (newval), "a" (oldval)
        : "memory"
    );
    return result == oldval;
}
```

###### 代码解释

- `lock cmpxchgl %2, %1`：`cmpxchgl` 是 x86 架构用于比较并交换的指令，`lock` 前缀确保该操作是原子性的。`%2` 代表新值，`%1` 代表要操作的内存地址。
- `"=a" (result)`：`"=a"`表明使用`EAX`寄存器作为输出，`result`是 C 语言变量，操作结束后，`EAX`的值会被存入`result`。`=`表示这是一个只写操作数，该值是比较并交换前内存中的值；是第一个输出操作数，编号为`%0`。
- `"+m" (*ptr)`：`"+m"`意味着使用内存地址`*ptr`作为输入输出操作数，`+`表示这是一个读写操作数，也就是该内存地址的值可能会被修改表示对内存地址 `ptr` 进行读写操作；是第二个输出操作数，编号为`%1`。所以`%1`代表的就是内存地址`*ptr`。
- `"r" (newval)`：把新值 `newval` 放入一个通用寄存器；由于前面已经有两个输出操作数，所以它是第三个操作数，编号为`%2`。因此`%2`代表的是`newval`。
- `"a" (oldval)`：将预期原值 `oldval` 放入 `eax` 寄存器；是第四个操作数，编号为`%3`。

##### 举例

下面以实现一个简单的原子计数器和无锁栈项目为例

###### 实现原子计数器

原子计数器可以在多线程环境下被多个线程同时访问并安全地增加计数值，使用 CAS 操作保证计数的原子性。

```
#include <stdio.h>
#include <pthread.h>

// 定义 CAS 操作函数
int compare_and_swap(int *ptr, int oldval, int newval) {
    int result;
    c
    return result == oldval;
}

// 全局计数器
int counter = 0;

// 线程函数，用于增加计数器值
void* increment_counter(void* arg) {
    for (int i = 0; i < 100000; i++) {
        int old_value;
        int new_value;
        do {
            old_value = counter;
            new_value = old_value + 1;
        } while (!compare_and_swap(&counter, old_value, new_value));
    }
    return NULL;
}

int main() {
    pthread_t threads[10];
    // 创建多个线程
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, increment_counter, NULL);
    }
    // 等待所有线程执行完毕
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    // 输出最终计数器值
    printf("Final counter value: %d\n", counter);
    return 0;
}    
```

1. **`compare_and_swap` 函数**：使用 `asm volatile` 内联汇编实现了 CAS 操作。`lock cmpxchgl` 指令保证了操作的原子性。
2. `while (!compare_and_swap(&stack->top, old_top, new_top));`：使用 cas 函数（即 CAS 操作）尝试将`counter`更新为 `newval`。如果更新失败（即其他线程在这期间修改了counter的值，则重新获取旧值和+1的新值，再次尝试更新。
3. **`increment_counter` 函数**：在循环中不断尝试使用 CAS 操作增加计数器的值，若操作失败则继续重试。
4. **`main` 函数**：创建多个线程同时对计数器进行增加操作，等待所有线程执行完毕后输出最终的计数器值。
5. **`cmpxchg` 和 `cmpxchgl` 影响结果**：在编程中我发现，如果用`cmpxchg`则无法实现原子操作count=100000，使用`cmpxchgl`正常实现，原因如下：
   - 若 cmpxchg 默认操作 64 位数据，而 counter 只是 32 位的，那么该指令可能会访问到 counter 变量之外的内存区域，这会导致不可预期的结果。比如，它可能会把 counter 变量旁边的内存数据也包含进来进行比较和交换操作，从而破坏了其他数据或者产生错误的比较结果。
   - 所以**汇编语言要严格注意变量的声明形状**。

###### 实现无锁栈

无锁栈允许多个线程同时进行入栈和出栈操作，使用 CAS 操作保证栈操作的线程安全。

```
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// 定义栈节点结构
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// 定义无锁栈结构
typedef struct {
    Node* top;
} LockFreeStack;

// 定义 CAS 操作函数
int compare_and_swap(Node** ptr, Node* oldval, Node* newval) {
    Node* result;
    __asm__ volatile (
        "lock cmpxchgq %2, %1"
        : "=a" (result), "+m" (*ptr)
        : "r" (newval), "a" (oldval)
        : "memory"
    );
    return result == oldval;
}

// 初始化无锁栈
void init_stack(LockFreeStack* stack) {
    stack->top = NULL;
}

// 入栈操作
void push(LockFreeStack* stack, int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    Node* old_top;
    do {
        old_top = stack->top;
        new_node->next = old_top;
    } while (!compare_and_swap(&stack->top, old_top, new_node));
}

// 出栈操作
int pop(LockFreeStack* stack) {
    Node* old_top;
    Node* new_top;
    do {
        old_top = stack->top;
        if (old_top == NULL) {
            return -1; // 栈为空
        }
        new_top = old_top->next;
    } while (!compare_and_swap(&stack->top, old_top, new_top));
    int data = old_top->data;
    free(old_top);
    return data;
}

// 线程函数，进行入栈操作
void* push_thread(void* arg) {
    LockFreeStack* stack = (LockFreeStack*)arg;
    for (int i = 0; i < 1000; i++) {
        push(stack, i);
    }
    return NULL;
}

// 线程函数，进行出栈操作
void* pop_thread(void* arg) {
    LockFreeStack* stack = (LockFreeStack*)arg;
    for (int i = 0; i < 1000; i++) {
        pop(stack);
    }
    return NULL;
}

int main() {
    LockFreeStack stack;
    init_stack(&stack);

    pthread_t push_threads[5];
    pthread_t pop_threads[5];

    // 创建入栈线程
    for (int i = 0; i < 5; i++) {
        pthread_create(&push_threads[i], NULL, push_thread, &stack);
    }
    // 创建出栈线程
    for (int i = 0; i < 5; i++) {
        pthread_create(&pop_threads[i], NULL, pop_thread, &stack);
    }

    // 等待所有线程执行完毕
    for (int i = 0; i < 5; i++) {
        pthread_join(push_threads[i], NULL);
        pthread_join(pop_threads[i], NULL);
    }

    return 0;
}    
```

1. **`compare_and_swap` 函数**：使用 `asm volatile` 内联汇编实现了针对指针的 CAS 操作，`lock cmpxchgq` 指令用于 64 位系统保证操作的原子性。

   - **`cmpxchgl`**：`l` 是长度前缀，明确指定该指令操作 32 位（4 字节）的数据。这里的 `l` 通常代表 “long”，在 x86 架构里，“long” 整数一般是 32 位。

   - **`cmpxchgq`**：`q` 同样是长度前缀，它指定该指令操作 64 位（8 字节）的数据。`q` 通常代表 “quad-word”，一个 “quad-word” 就是 64 位。

2. **`push` 函数**：创建新节点并使用 CAS 操作将其插入栈顶，若操作失败则重试。

3. **`pop` 函数**：使用 CAS 操作移除栈顶节点并返回其值，若栈为空则返回 -1。

4. **`main` 函数**：创建多个入栈和出栈线程，模拟多线程环境下对栈的并发操作。

##### 问题：

1. 原子操作真的需要用循环来判断是否插入成功吗，如果必须用循环判断还是原子操作吗？（写个测试看看有没有插入失败的情况出现）

### 2 线程池的实现

 线程池是一种多线程处理技术，它维护着一个线程集合，这些线程可以被重复使用来执行多个任务，而不是为每个任务都创建一个新线程。线程池由三个部分组成：

1. 任务队列
2. 执行队列
3. 管理组件--锁

![image-20250413193043614](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250413193043614.png)

#### 1 线程池的链表结构体实现

##### 任务队列：

```
struct nTask{
    void (*task_func)(struct nTask *task);
    void *user_data;
    struct nTask *next;
    struct nTask *prev;
};
```

- 线程池回调函数指针类型的定义`void (*task_func)(struct nTask *task)`，回调函数要用括号括上
- `typedef`可以简化应用名，直接当做一个类型来使用，如果不是用的话，也可以正常赋值使用
- 数据指针使用空类型指针，因为不知道会使用什么类型
- 链表前后指针

##### 执行队列：

```
struct nWorker{
    pthread_t threadid;
    int terminate;
    struct nManager *manager;
    struct nWorker *next;
    struct nWorker *prev;
};
```

- 线程ID（？源代码里没有初始化线程id，原因：&worker->threadid：是一个指向 pthread_t 类型变量的指针，pthread_create 函数会将新创建线程的 ID 存储到这个变量中）
- 是否销毁变量
- 管理（线程池）指针
- 前后指针

##### 管理组件：

```
typedef struct nManager{
    struct nTask *tasks;
    struct nWorker *workers;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ThreadPool;
```

- 任务队列
- 执行队列
- 互斥锁`pthread_mutex_t`
- 线程条件变量`pthread_cond_t`

##### 线程池回调函数（线程启动后要执行的函数）：

```
static void *nThreadPoolCallBack(void *arg){
    /*
    struct nWorker *worker = (struct nWorker *)malloc(sizeof(struct nWorker));
    worker = (struct nWorker *)arg;
    */
    //这种写法会导致内存泄露，实际上堆上分配的内存并没有被使用,应该使用以下写法
    struct nWorker *worker = (struct nWorker *)arg;

    while(1){
        pthread_mutex_lock(&worker->manager->mutex);

        while(worker->manager->tasks == NULL){
            if(worker->terminate){
                break;
            }
            
            pthread_cond_wait(&worker->manager->cond, &worker->manager->mutex);
            
        }
        if(worker->terminate){
            pthread_mutex_unlock(&worker->manager->mutex);
            break;
        }
        struct nTask *task = worker->manager->tasks;
        LIST_REMOVE(task, worker->manager->tasks);

        pthread_mutex_unlock(&worker->manager->mutex);
        task->task_func(task);
    }

    free(worker);
}
```

- `static`使用内部链接，不会被其他源文件引用，
- 回调函数不等于任务，只是在循环中判断，如果没有任务，就持续等待，如果有任务，就把读取并在业务队列中删除，然后执行任务函数，最后退出执行队列
- 条件等待`pthread_cond_wait(&pthread_cond_t，&pthread_mutex_t)`，主要作用是让线程在循环中等待
- 当 `pthread_create` 函数成功返回时，worker->threadid 就会被赋值为新创建线程的唯一 ID，而不是 0。
- 功能实现：条件变量`pthread_cond_wait`是专门为线程间的条件等待和通知机制设计的。它会自动释放锁，并将线程置于等待状态，直到收到其他线程通过`pthread_cond_signal`或`pthread_cond_broadcast`发送的通知。而仅用条件语句，比如`while(condition)`，线程会一直循环检查条件，不会释放锁，也不会进入等待状态，这会导致其他需要获取该锁的线程无法执行，造成资源浪费和程序效率低下。 
- **死锁风险**：如果只用条件语句实现等待，且没有合适的机制来更新条件变量，很容易导致死锁。因为线程会一直持有锁，其他线程无法修改条件，从而使等待的线程永远无法满足条件继续执行。而使用`pthread_cond_wait`，线程在等待时会释放锁，允许其他线程获取锁并修改条件，避免了死锁的发生。
- `pthread_mutex_lock(&worker->manager->mutex);`： 加锁操作是为了获取对线程池的互斥锁。由于任务队列 worker->manager->tasks 是多个工作线程共享的资源，多个线程可能同时尝试访问或修改它。加锁可以确保在同一时刻只有一个线程能够访问和操作任务队列，防止出现数据竞争和不一致的情况。例如，如果多个线程同时读取或修改任务队列，可能会导致数据混乱或错误。 
- `while (worker->manager->tasks == NULL) {... }` 循环： 这个循环用于检查任务队列是否为空。如果任务队列为空，线程需要等待新任务的到来。在检查任务队列之前加锁，是为了确保在检查过程中任务队列不会被其他线程修改。 
- `pthread_cond_wait(&worker->manager->cond, &worker->manager->mutex);`： 当任务队列为空时，线程调用 `pthread_cond_wait` 函数进入等待状态。`pthread_cond_wait` 函数会自动释放互斥锁（这是它的一个重要特性），这样其他线程就可以获取锁并向任务队列中添加新任务。当条件变量被其他线程通过 `pthread_cond_signal` 或 `pthread_cond_broadcast` 唤醒时，`pthread_cond_wait` 函数会重新获取互斥锁，然后线程继续执行。 
- `struct nTask *task = worker->manager->tasks;` 和 `LIST_REMOVE(task, worker->manager->tasks);`： 从任务队列中取出一个任务。在取出任务之前加锁，是为了保证任务队列的一致性，防止其他线程在此时修改任务队列。
- `pthread_mutex_unlock(&worker->manager->mutex);`： 解锁操作是为了释放对任务队列的独占访问权，允许其他线程获取锁并访问任务队列。这样可以提高程序的并发性能，让其他线程能够继续执行相关操作，如添加任务或取出任务。 
- `task->task_func(task);`： 执行任务的处理函数。此时互斥锁已经被释放，线程可以在不影响其他线程对任务队列操作的情况下执行任务。 
- `free(worker);`： 当线程完成任务处理或线程需要终止时，释放线程相关的资源。
- 当线程调用 `pthread_cond_wait` 时，会执行以下操作： 
  - **释放互斥锁**：线程会自动释放传入的互斥锁 `mutex`。这是为了允许其他线程能够获取该互斥锁，从而可以修改共享资源和发送条件变量信号。 
  - **进入等待状态**：线程会进入阻塞状态，等待条件变量 cond 被其他线程通过 `pthread_cond_signal` 或 `pthread_cond_broadcast` 函数发出信号。
  -  **重新获取互斥锁**：当收到条件变量的信号后，线程会从等待状态中唤醒，并且会尝试重新获取之前释放的互斥锁 mutex。只有当成功获取到互斥锁后，线程才会继续执行 `pthread_cond_wait` 调用之后的代码。
##### 线程池创建：

```
int nThreadPoolCreate(ThreadPool *pool, int nWorker){
    if(pool == NULL) return -1;
    if(nWorker < 1) nWorker = 1;
    memset(pool, 0, sizeof(ThreadPool));

    //pthread_mutex_init(&pool->mutex, NULL);
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&pool->mutex, &blank_mutex, sizeof(pthread_mutex_t));
    
    //pthread_cond_init(&pool->cond, NULL);
    pthread_cond_t blank_cond= PTHREAD_COND_INITIALIZER;
    memcpy(&pool->cond, &blank_cond, sizeof(pthread_cond_t));

    for (int i = 0; i < nWorker; i++){
        struct nWorker *worker = (struct nWorker *)malloc(sizeof(struct nWorker));
        if (worker == NULL){
            perror("malloc");
            return -2;
        }
        memset(worker, 0, sizeof(struct nWorker));
        worker->manager = pool;

        int ret = pthread_create(&worker->threadid, NULL, nThreadPoolCallBack, worker);
        if(ret){
            perror("pthread_create");
            free(worker);
            return -3;
        }

        LIST_INSERT(worker, pool->workers);
    }

    return 0;
}
```

- `pthread_cond_t` 是 POSIX 线程库（pthread）里用于实现条件变量的类型。条件变量是一种同步原语，它能让线程以线程安全的方式等待某个条件成真。在多线程编程中，条件变量常和互斥锁配合使用，以此确保线程在等待条件时不会出现竞态条件。
- PTHREAD_COND_INITIALIZER 是一个宏，其用途是对静态分配的 pthread_cond_t 变量进行初始化。当你在程序启动之前就已经知道条件变量的存在，并且它属于全局或者静态变量时，就可以使用这个宏来进行初始化。
- memset(pool, 0, sizeof(ThreadPool));：memset 函数的作用是把一块内存区域初始化为指定的值。它的第一个参数要求是一个指向要初始化的内存块的指针.
- 在 perror("malloc"); 这行代码中，当 malloc 函数调用失败（返回 NULL）时，perror 函数会打印出类似于 “malloc: [具体的错误描述]” 的信息
- 与一些简单的内存分配方式（如使用自动变量，即在函数内部直接声明变量，它们存储在栈上）相比，malloc 适用于需要在函数调用结束后仍然保留数据（因为栈上的自动变量在函数返回时会被自动释放）或者内存大小在编译时无法确定的情况。
- 注意把创建的线程置为空`memset(pool, 0, sizeof(ThreadPool));`

##### 线程池销毁：

```
int nThreadPoolDestroy(ThreadPool *pool, int nWorker){
    struct nWorker *worker = NULL;
    for(worker = pool->workers; worker != NULL; worker = worker->next){
        worker->terminate = 1;
    }
    
    pthread_mutex_lock(&pool->mutex);

    pthread_cond_broadcast(&pool->cond);

    pthread_mutex_unlock(&pool->mutex);

    // 等待所有线程结束
    for (worker = pool->workers; worker != NULL; worker = worker->next) {
        pthread_join(worker->threadid, NULL);
    }

    pool->workers = NULL;
    pool->tasks = NULL;

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    return 0;
}
```

- `pthread_cond_broadcast` 是 `POSIX` 线程库中的一个函数，其作用是唤醒所有正在等待指定条件变量的线程。
- 这个线程池的销毁我实在没看懂在做什么，需要**提问**！
- **保护共享资源**：线程池中的任务队列（`pool->tasks`）和工作线程列表（`pool->workers`）属于共享资源，多个工作线程可能会同时访问和修改这些资源。在调用 `pthread_cond_broadcast` 之前加锁，能够保证在广播条件变量时，这些共享资源不会被其他线程修改。
- **避免竞态条件**：若不加锁，可能会出现这样的情况：一个工作线程正准备进入等待状态（调用 `pthread_cond_wait`），而此时另一个线程调用 `pthread_cond_broadcast` 进行广播，这样该工作线程就会错过广播信号，从而导致线程无法正确终止。加锁可以确保在广播时，所有工作线程的状态是稳定的，避免出现竞态条件。
- 使用 `pthread_join` 等待所有线程结束

##### 线程池添加任务：

```
int nThreadPoolPushTask(ThreadPool *pool, struct nTask *task){
    

    pthread_mutex_lock(&pool->mutex);

    LIST_INSERT(task, pool->tasks);

    pthread_cond_signal(&pool->cond);
    
    pthread_mutex_unlock(&pool->mutex);

    return 0;
}
```

- `pthread_cond_signal`：该函数用于唤醒在指定条件变量上等待的至少一个线程。若有多个线程在等待，系统会选择其中一个线程唤醒。
  `pthread_cond_broadcast`：此函数会唤醒在指定条件变量上等待的所有线程。
- 函数的作用是把任务分配给线程池，注意是**先添加任务，再唤醒线程**
- `pthread_mutex_lock(&pool->mutex);`：使用`pthread_mutex_lock`函数对互斥锁`pool->mutex`进行加锁。这是为了确保在对任务队列进行操作时的线程安全性，防止多个线程同时访问和修改任务队列，避免出现数据竞争等问题。
- `LIST_INSERT(task, pool->tasks);`：通过自定义的`LIST_INSERT`宏将任务`task`插入到任务队列`pool->tasks`中。该宏会将新任务插入到队列头部，更新相关指针，使得任务被正确添加到等待处理的任务列表中。
- `pthread_cond_signal(&pool->cond);`：调用`pthread_cond_signal`函数发送信号给条件变量`pool->cond`。这会唤醒正在等待该条件变量的一个线程，通知它任务队列中有新任务可以处理了。之所以在加锁状态下发送信号，是为了保证在更新任务队列后，线程能够正确地获取到新任务。如果先解锁再发送信号，可能会出现刚解锁，其他线程就修改了任务队列，导致被唤醒的线程获取到错误的任务信息或者错过新任务的情况。
- `pthread_mutex_unlock(&pool->mutex);`：使用`pthread_mutex_unlock`函数对互斥锁进行解锁，释放对任务队列的独占访问权，允许其他线程继续访问和操作任务队列。这样，其他线程就可以继续添加任务到队列中，或者等待获取任务进行处理。

##### SDK调试代码

```
// sdk --> debug thread pool

#if 1

#define THREADPOOL_INIT_COUNT	20
#define TASK_INIT_SIZE			1000


void task_entry(struct nTask *task) { //type 

	//struct nTask *task = (struct nTask*)task;
	int idx = *(int *)task->user_data;

	INFO("idx: %d\n", idx);

	free(task->user_data);
	free(task);
}


int main(void) {

	ThreadPool pool = {0};
	
	nThreadPoolCreate(&pool, THREADPOOL_INIT_COUNT);
	// pool --> memset();
	
	int i = 0;
	for (i = 0;i < TASK_INIT_SIZE;i ++) {
		struct nTask *task = (struct nTask *)malloc(sizeof(struct nTask));
		if (task == NULL) {
			perror("malloc");
			exit(1);
		}
		memset(task, 0, sizeof(struct nTask));

		task->task_func = task_entry;
		task->user_data = malloc(sizeof(int));
		*(int*)task->user_data  = i;

		
		nThreadPoolPushTask(&pool, task);
	}

	nThreadPoolDestroy(&pool, THREADPOOL_INIT_COUNT);
	
}


#endif
```

- 注意malloc需要进行类型转换，在c中可以进行隐式类型转换，在c++中只能进行显式类型转换
- 最后用pthread_join来等待线程结束更好

##### 调试工具`gdb`

- 首先在gcc编译时加入`-g`,在使用 gdb 调试程序之前，需要在编译时加上 -g 选项，这样编译器会在可执行文件中加入调试信息
- `gdb program`启动gdb

- `b -行数`加入断点

###### 运行程序

- run（或 r）：开始运行程序。
- start：开始执行程序，在主函数的起始处停止。

###### 设置断点

- break <行号>（或 b <行号>）：在指定行号处设置断点。
- break <函数名>（或 b <函数名>）：在指定函数的入口处设置断点。
- delete <断点编号>（或 d <断点编号>）：删除指定编号的断点。

###### 单步执行

- next（或 n）：单步执行下一行代码，不进入函数内部。

- step（或 s）：单步执行下一行代码，如果是函数调用则进入函数内部。
- continue（或 c）：继续执行程序，直到下一个断点。

###### 查看变量

- print <变量名>（或 p <变量名>）：打印指定变量的值。
- display <变量名>：每次程序停止时自动显示指定变量的值。
- undisplay <显示编号>：取消指定编号的自动显示。

###### 查看栈信息

- backtrace（或 bt）：查看函数调用栈信息。
- frame <栈帧编号>：切换到指定的栈帧。

###### 退出 gdb

- quit（或 q）：退出 gdb。

> 通过gdb调试了三个问题,b,r,c
> 1. pool --> memset()，内存没有置为空导致读错数
> 2. void *arg类型定义错误，导致指针偏移
> 3. 3.主线程没有等待任务的结束。getchar()
> 4. 

### 完整代码

```
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>


#define INFO printf

#define LIST_INSERT(item, list) do {          \
    (item)->prev = NULL;                      \
    (item)->next = list;                      \
    if ((list) != NULL) (list)->prev = (item);\
    (list) = (item);                          \
}while(0)

#define LIST_REMOVE(item, list) do {                           \
    if((item)->prev != NULL) (item)->prev->next = (item)->next;\
    if((item)->next != NULL) (item)->next->prev = (item)->prev;\
    if((item) == (list)) (list) = (item)->next;                \
    (item)->next = (item)->prev = NULL;                        \
}while(0)

struct nTask{
    void (*task_func)(struct nTask *task);
    void *user_data;
    struct nTask *next;
    struct nTask *prev;
};

struct nWorker{
    pthread_t threadid;
    int terminate;
    struct nManager *manager;
    struct nWorker *next;
    struct nWorker *prev;
};

typedef struct nManager{
    struct nTask *tasks;
    struct nWorker *workers;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} ThreadPool;

static void *nThreadPoolCallBack(void *arg){
    /*
    struct nWorker *worker = (struct nWorker *)malloc(sizeof(struct nWorker));
    worker = (struct nWorker *)arg;
    */
    //这种写法会导致内存泄露，实际上堆上分配的内存并没有被使用,应该使用以下写法
    struct nWorker *worker = (struct nWorker *)arg;

    while(1){
        pthread_mutex_lock(&worker->manager->mutex);

        while(worker->manager->tasks == NULL){
            if(worker->terminate){
                break;
            }
            
            pthread_cond_wait(&worker->manager->cond, &worker->manager->mutex);
            
        }
        if(worker->terminate){
            pthread_mutex_unlock(&worker->manager->mutex);
            break;
        }
        struct nTask *task = worker->manager->tasks;
        LIST_REMOVE(task, worker->manager->tasks);

        pthread_mutex_unlock(&worker->manager->mutex);
        task->task_func(task);
    }

    free(worker);
}

int nThreadPoolCreate(ThreadPool *pool, int nWorker){
    if(pool == NULL) return -1;
    if(nWorker < 1) nWorker = 1;
    memset(pool, 0, sizeof(ThreadPool));

    //pthread_mutex_init(&pool->mutex, NULL);
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&pool->mutex, &blank_mutex, sizeof(pthread_mutex_t));
    
    //pthread_cond_init(&pool->cond, NULL);
    pthread_cond_t blank_cond= PTHREAD_COND_INITIALIZER;
    memcpy(&pool->cond, &blank_cond, sizeof(pthread_cond_t));

    for (int i = 0; i < nWorker; i++){
        struct nWorker *worker = (struct nWorker *)malloc(sizeof(struct nWorker));
        if (worker == NULL){
            perror("malloc");
            return -2;
        }
        memset(worker, 0, sizeof(struct nWorker));
        worker->manager = pool;

        int ret = pthread_create(&worker->threadid, NULL, nThreadPoolCallBack, worker);
        if(ret != 0){
            perror("pthread_create");
            free(worker);
            return -3;
        }

        LIST_INSERT(worker, pool->workers);
    }

    return 0;
}

int nThreadPoolDestroy(ThreadPool *pool, int nWorker){
    struct nWorker *worker = NULL;
    for(worker = pool->workers; worker != NULL; worker = worker->next){
        worker->terminate = 1;
    }
    
    pthread_mutex_lock(&pool->mutex);

    pthread_cond_broadcast(&pool->cond);

    pthread_mutex_unlock(&pool->mutex);

    // 等待所有线程结束
    for (worker = pool->workers; worker != NULL; worker = worker->next) {
        pthread_join(worker->threadid, NULL);
    }

    pool->workers = NULL;
    pool->tasks = NULL;

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    return 0;
}

int nThreadPoolPushTask(ThreadPool *pool, struct nTask *task){
    

    pthread_mutex_lock(&pool->mutex);

    LIST_INSERT(task, pool->tasks);

    pthread_cond_signal(&pool->cond);
    
    pthread_mutex_unlock(&pool->mutex);

    return 0;
}

// sdk --> debug thread pool

#if 1

#define THREADPOOL_INIT_COUNT	20
#define TASK_INIT_SIZE			1000


void task_entry(struct nTask *task) { //type 

	//struct nTask *task = (struct nTask*)task;
	int idx = *(int *)task->user_data;

	INFO("idx: %d\n", idx);

	free(task->user_data);
	free(task);
}


int main(void) {

	ThreadPool pool = {0};
	
	nThreadPoolCreate(&pool, THREADPOOL_INIT_COUNT);
	// pool --> memset();
	
	int i = 0;
	for (i = 0;i < TASK_INIT_SIZE;i ++) {
		struct nTask *task = (struct nTask *)malloc(sizeof(struct nTask));
		if (task == NULL) {
			perror("malloc");
			exit(1);
		}
		memset(task, 0, sizeof(struct nTask));

		task->task_func = task_entry;
		task->user_data = malloc(sizeof(int));
		*(int*)task->user_data  = i;

		
		nThreadPoolPushTask(&pool, task);
	}

	nThreadPoolDestroy(&pool, THREADPOOL_INIT_COUNT);
	return 0;
}


#endif
```

