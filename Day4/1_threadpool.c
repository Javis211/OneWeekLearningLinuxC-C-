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