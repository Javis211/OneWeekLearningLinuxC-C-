#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <pthread.h>

#define HSY_DB_SERVER_IP   "192.168.21.129"
#define HSY_DB_SERVER_PORT 3306

#define HSY_DB_USERNAME    "admin"
#define HSY_DB_PASSWORD    "cv"
#define HSY_DB_DEFAULTDB   "HSY_DB"
// C U R D
#define SQL_INSERT_TBL_USER "INSERT INTO TBL_USER(U_NAME, U_GENDER) VALUES('ikun', 'man');"
#define SQL_SELECT_TBL_USER "SELECT * FROM TBL_USER;"
#define SQL_DELETE_TBL_USER "CALL PROC_DELETE_USER('ikun')"

#define SQL_INSERT_IMG_USER "INSERT TBL_USER(U_NAME, U_GENDER, U_IMAGE) VALUES('ikun', 'man', ?);"//问号作为占位符
#define SQL_SELECT_IMG_USER	"SELECT U_IMAGE FROM TBL_USER WHERE U_NAME='ikun';"

#define FILE_IMAGE_LENGTH (64*1024)

/*--------------------------------------------------------*/
// 连接池
#define CONNECTION_POOL_SIZE 5

// 连接池结构体
typedef struct {
    MYSQL *connections[CONNECTION_POOL_SIZE]; //connections：一个数组，用于存储多个 MYSQL 连接对象。
    int used[CONNECTION_POOL_SIZE]; // used：一个数组，用于标记每个连接是否正在被使用。
    pthread_mutex_t lock; // lock：一个互斥锁，用于保证多线程环境下对连接池的操作是线程安全的。
} ConnectionPool;

// 初始化连接池
void init_connection_pool(ConnectionPool *pool) {
    pthread_mutex_init(&pool->lock, NULL);
    // 循环创建指定数量的 MYSQL 连接，并尝试连接到数据库。
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        pool->connections[i] = mysql_init(NULL);
        if (pool->connections[i] == NULL) {
            printf("mysql_init : %s\n", mysql_error(pool->connections[i]));
            continue;
        }
        if (mysql_real_connect(pool->connections[i], HSY_DB_SERVER_IP, HSY_DB_USERNAME,
                               HSY_DB_PASSWORD, HSY_DB_DEFAULTDB, HSY_DB_SERVER_PORT, NULL, 0) == NULL) {
            printf("mysql_connect: %s\n", mysql_error(pool->connections[i]));
            mysql_close(pool->connections[i]);
            pool->connections[i] = NULL;
        } else {
            pool->used[i] = 0; // 如果连接成功，将 used 数组对应位置标记为未使用（0）
        }
    }
}

// 从连接池获取连接
MYSQL *get_connection(ConnectionPool *pool) {
    pthread_mutex_lock(&pool->lock); // 加锁，保证线程安全。
    // 遍历 used 数组，找到未使用的连接。
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        if (!pool->used[i]) {
            pool->used[i] = 1;
            pthread_mutex_unlock(&pool->lock);
            return pool->connections[i];
        } // 解锁并返回该连接。
    }
    pthread_mutex_unlock(&pool->lock);
    return NULL;
}

// 归还连接到连接池
void release_connection(ConnectionPool *pool, MYSQL *conn) {
    pthread_mutex_lock(&pool->lock);
    // 遍历 connections 数组，找到要归还的连接。
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        if (pool->connections[i] == conn) {
            pool->used[i] = 0;
            break;
        }
    }
    // 将其对应的 used 数组位置标记为未使用（0），解锁
    pthread_mutex_unlock(&pool->lock);
}

// 销毁连接池
void destroy_connection_pool(ConnectionPool *pool) {
    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        if (pool->connections[i] != NULL) {
            mysql_close(pool->connections[i]);
        }
    }
    pthread_mutex_unlock(&pool->lock);
    pthread_mutex_destroy(&pool->lock);
}

// 定义全局变量用于线程同步
ConnectionPool pool;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int write_completed = 0;

// 写入数据库的线程函数
void* write_to_database(void* arg) {
    MYSQL *conn = get_connection(&pool);
    if (conn == NULL) {
        printf("get_connection: %s\n", mysql_error(conn));
        pthread_exit(NULL);
    }

    char buffer[FILE_IMAGE_LENGTH] = {0};
    int length = read_image(arg, buffer);
    if (length < 0) {
        release_connection(&pool, conn);
        pthread_exit(NULL);
    }

    if (mysql_write(conn, buffer, length) < 0) {
        release_connection(&pool, conn);
        pthread_exit(NULL);
    }

    // 写入完成，加锁并设置标志位，然后通知其他线程
    pthread_mutex_lock(&mutex);
    write_completed = 1;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    release_connection(&pool, conn);
    pthread_exit(NULL);
}

// 从数据库读取并写入图片的线程函数
void* read_from_database_and_write_image(void* arg) {
    // 加锁等待写入完成
    pthread_mutex_lock(&mutex);
    while (!write_completed) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    MYSQL *conn = get_connection(&pool);
    if (conn == NULL) {
        printf("get_connection: %s\n", mysql_error(conn));
        pthread_exit(NULL);
    }

    char buffer[FILE_IMAGE_LENGTH] = {0};
    int length = mysql_read(conn, buffer, FILE_IMAGE_LENGTH);
    if (length < 0) {
        release_connection(&pool, conn);
        pthread_exit(NULL);
    }

    if (write_image("a1.jpg", buffer, length) < 0) {
        release_connection(&pool, conn);
        pthread_exit(NULL);
    }

    release_connection(&pool, conn);
    pthread_exit(NULL);
}
/*--------------------------------------------------------*/

int hsy_mysql_select(MYSQL *handle){
    // mysql_real_query 查询数据库
    if(0 != mysql_real_query(handle, SQL_SELECT_TBL_USER, strlen(SQL_SELECT_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(handle));
        return -1;
    }
    // 从存储 store 获取结果
    MYSQL_RES *res = mysql_store_result(handle);
    if (res == NULL){
        printf("mysql_store_result: %s\n", mysql_error(handle));
        return -2;
    }
    // num: rows / fields 输出行列数
    int rows = mysql_num_rows(res);
    printf("rows: %d\n", rows);

    int fields = mysql_num_fields(res);
    printf("fields: %d\n", fields);

    // fetch: 输出行列具体信息
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res))){
        for (int i = 0; i < fields; i ++){
            printf("%s\t", row[i]);
        }
        printf("\n");
    }
    
    mysql_free_result(res);

    return 0;
}

//读取图片
int read_image(char *filename, char *buffer){
    if (filename == NULL || buffer == NULL) return -1;

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("fopen failed!\n");
        return -2;
    }

    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int size = fread(buffer, 1, length, fp);
    if (size != length){
        printf("fread failed!\n");
        return -3;
    }
    
    fclose(fp);

    return size;
}

//写入磁盘
int write_image(char *filename, char *buffer, int length){
    if (filename == NULL || buffer == NULL || length <= 0) return -1;

    FILE *fp = fopen(filename, "wb+");
    if (fp == NULL){
        printf("%s fopen wb+ failed!\n", filename);
        return -2;
    }

    int size = fwrite(buffer, 1, length, fp);
    if (size != length){
        printf("fwrite failed: %d\n", size);
        return -3;
    }

    fclose(fp);

    return size;
}

//写入数据库
int mysql_write(MYSQL *handle, char *buffer, int length){
    if (handle == NULL || buffer == NULL || length <= 0) return -1;

    MYSQL_STMT *stmt = mysql_stmt_init(handle);
    int ret = mysql_stmt_prepare(stmt, SQL_INSERT_IMG_USER, sizeof(SQL_INSERT_IMG_USER));
    if(ret){
        printf("mysql_stmt_prepare: %s\n", mysql_error(handle));
        return -2;
    }

    MYSQL_BIND param = {0};
    param.buffer_type  = MYSQL_TYPE_LONG_BLOB;
	param.buffer = NULL;
	param.is_null = 0;
	param.length = NULL;

    ret = mysql_stmt_bind_param(stmt, &param);
    if(ret){
        printf("mysql_stmt_bind_param: %s\n", mysql_error(handle));
        return -3;
    }

    ret = mysql_stmt_send_long_data(stmt, 0, buffer, length);
    if(ret){
        printf("mysql_stmt_send_long_data: %s\n", mysql_error(handle));
        return -4;
    }

    ret = mysql_stmt_execute(stmt);
    if(ret){
        printf("mysql_stmt_execute: %s\n", mysql_error(handle));
        return -5;
    }

    ret = mysql_stmt_close(stmt);
    if(ret){
        printf("mysql_stmt_close: %s\n", mysql_error(handle));
        return -6;
    }

    return ret;
}

//读取数据库
// 函数功能：从 MySQL 数据库中读取数据到缓冲区
// 参数说明：
// handle：指向已经初始化并连接到 MySQL 服务器的 MYSQL 对象的指针
// buffer：用于存储从数据库中读取数据的缓冲区指针
// length：缓冲区的长度
int mysql_read(MYSQL *handle, char *buffer, int length) {

    // 检查传入的参数是否有效，如果 handle 为空、buffer 为空或者 length 小于等于 0，则返回 -1 表示错误
    if (handle == NULL || buffer == NULL || length <= 0) return -1;

    // 初始化一个预处理语句对象，用于执行 SQL 查询语句
    MYSQL_STMT *stmt = mysql_stmt_init(handle);

    // 准备 SQL 语句，这里 SQL_SELECT_IMG_USER 是一个宏定义的 SQL 查询语句
    // strlen(SQL_SELECT_IMG_USER) 计算该 SQL 语句的长度
    int ret = mysql_stmt_prepare(stmt, SQL_SELECT_IMG_USER, strlen(SQL_SELECT_IMG_USER));
    if (ret) {
        // 如果预处理语句准备失败，打印错误信息并返回 -2 表示错误
        printf("mysql_stmt_prepare : %s\n", mysql_error(handle));
        return -2;
    }

    // 初始化一个 MYSQL_BIND 结构体，用于绑定查询结果的列到变量
    MYSQL_BIND result = {0};

    // 设置要读取的列的数据类型为 LONG BLOB
    result.buffer_type = MYSQL_TYPE_LONG_BLOB;
    // 用于存储读取到的数据的总长度
    unsigned long total_length = 0;
    // 将 total_length 的地址赋给 result.length，以便后续获取数据长度
    result.length = &total_length;

    // 将查询结果的列绑定到 result 变量上
    ret = mysql_stmt_bind_result(stmt, &result);
    if (ret) {
        // 如果绑定结果失败，打印错误信息并返回 -3 表示错误
        printf("mysql_stmt_bind_result : %s\n", mysql_error(handle));
        return -3;
    }

    // 执行预处理的 SQL 语句
    ret = mysql_stmt_execute(stmt);
    if (ret) {
        // 如果执行 SQL 语句失败，打印错误信息并返回 -4 表示错误
        printf("mysql_stmt_execute : %s\n", mysql_error(handle));
        return -4;
    }

    // 从服务器读取查询结果集并存储到客户端
    ret = mysql_stmt_store_result(stmt);
    if (ret) {
        // 如果存储结果失败，打印错误信息并返回 -5 表示错误
        printf("mysql_stmt_store_result : %s\n", mysql_error(handle));
        return -5;
    }

    // 循环读取结果集中的数据
    while (1) {

        // 获取结果集中的下一行数据
        ret = mysql_stmt_fetch(stmt);
        // 如果读取失败且不是数据截断的情况（MYSQL_DATA_TRUNCATED），则跳出循环
        if (ret != 0 && ret != MYSQL_DATA_TRUNCATED) break; 

        int start = 0;
        // 循环将读取到的数据逐字节存储到 buffer 缓冲区中
        while (start < (int)total_length) {
            // 设置 buffer 的起始位置
            result.buffer = buffer + start;
            // 设置每次读取的字节数为 1
            result.buffer_length = 1;
            // 从指定列（这里是第 0 列）的指定偏移量（start）处读取数据
            mysql_stmt_fetch_column(stmt, &result, 0, start);
            // 更新偏移量
            start += result.buffer_length;
        }
    }

    // 关闭预处理语句对象，释放相关资源
    mysql_stmt_close(stmt);

    // 返回读取到的数据的总长度，如果发生错误会在前面的步骤中返回相应的错误码
    return total_length;
}

int main(){
    // 获取连接池
    ConnectionPool pool;
    init_connection_pool(&pool);
    MYSQL *conn = {0};

#if 1
    // insert
    // 示例：获取连接并执行查询
    conn = get_connection(&pool);
    if (conn == NULL) {
        printf("get_connection: %s\n", mysql_error(conn));
        goto Exit;
    }
    if(0 != mysql_real_query(conn, SQL_INSERT_TBL_USER, strlen(SQL_INSERT_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(conn));
        goto Exit;
    }

    hsy_mysql_select(conn);
    release_connection(&pool, conn);
#endif
    printf("case : mysql --> delete \n");
#if 1
    // delete
    // 示例：获取连接并执行查询
    conn = get_connection(&pool);
    if (conn == NULL) {
        printf("get_connection: %s\n", mysql_error(conn));
        goto Exit;
    }
    if(0 != mysql_real_query(conn, SQL_DELETE_TBL_USER, strlen(SQL_DELETE_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(conn));
        goto Exit;
    }

    hsy_mysql_select(conn);
    release_connection(&pool, conn);
#endif
#if 1
    // 示例：获取连接并执行查询
    conn = get_connection(&pool);
    if (conn == NULL) {
        printf("get_connection: %s\n", mysql_error(conn));
        goto Exit;
    }

    printf("case : mysql --> read image and write mysql\n");
	
	char buffer[FILE_IMAGE_LENGTH] = {0};
	int length = read_image("0voice.jpg", buffer);
	if (length < 0) goto Exit;
	
	mysql_write(conn, buffer, length); /// 


	printf("case : mysql --> read mysql and write image\n");
	
	memset(buffer, 0, FILE_IMAGE_LENGTH);
	length = mysql_read(conn, buffer, FILE_IMAGE_LENGTH);

	write_image("a1.jpg", buffer, length);

    release_connection(&pool, conn);
#endif


Exit:
    destroy_connection_pool(&pool);
    return 0;
}