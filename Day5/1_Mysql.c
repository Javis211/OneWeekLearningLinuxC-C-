#include <stdio.h>
#include <string.h>
#include <mysql.h>

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
    // 声明mysql
    MYSQL mysql; 

    // 初始化mysql
    if (NULL == mysql_init(&mysql)){
        printf("mysql_init : %s\n", mysql_error(&mysql));
        goto Exit;
    }

    // 连接mysql
    if (NULL == mysql_real_connect(&mysql, HSY_DB_SERVER_IP, HSY_DB_USERNAME,
         HSY_DB_PASSWORD, HSY_DB_DEFAULTDB, HSY_DB_SERVER_PORT, NULL, 0)){
        printf("mysql_connect: %s\n", mysql_error(&mysql));
        goto Exit;
    }

    printf("case : mysql --> insert \n");
#if 1
    // insert
    if(0 != mysql_real_query(&mysql, SQL_INSERT_TBL_USER, strlen(SQL_INSERT_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(&mysql));
        goto Exit;
    }
#endif
    hsy_mysql_select(&mysql);

    printf("case : mysql --> delete \n");
#if 1
    // delete
    if(0 != mysql_real_query(&mysql, SQL_DELETE_TBL_USER, strlen(SQL_DELETE_TBL_USER))){
        printf("mysql_real_query: %s\n", mysql_error(&mysql));
        goto Exit;
    }
#endif

    hsy_mysql_select(&mysql);

    printf("case : mysql --> read image and write mysql\n");
	
	char buffer[FILE_IMAGE_LENGTH] = {0};
	int length = read_image("0voice.jpg", buffer);
	if (length < 0) goto Exit;
	
	mysql_write(&mysql, buffer, length); /// 


	printf("case : mysql --> read mysql and write image\n");
	
	memset(buffer, 0, FILE_IMAGE_LENGTH);
	length = mysql_read(&mysql, buffer, FILE_IMAGE_LENGTH);

	write_image("a.jpg", buffer, length);

Exit:
	mysql_close(&mysql);

    return 0;
}