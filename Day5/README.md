# 【Linux入门环境编程】数据库MySQL

### 1 MySQL基本介绍

MySQL是最流行的关系型数据库管理系统,在WEB应用方面MySQL是最好的RDBMS(Relational Database Management System:关系数据库管理系统)应用软件之一。

#### 什么是数据库?

数据库(Database)是按照数据结构来组织、存储和管理数据的仓库。RDBMS即关系数据库管理系统(Relational Database Management System)的特点:

1. 数据以表格的形式出现

2. 每行为各种记录名称

3. 每列为记录名称所对应的数据域

4. 许多的行和列组成一张表单

5. 若干的表单组成database

#### RDBMS术语

在我们开始学习MySQL数据库前,让我们先了解下RDBMS的一些术语:

1. 数据库`database`:数据库是一些关联表的集合。

2. 数据表`table`:表是数据的矩阵。在一个数据库中的表看起来像一个简单的电子表格

3. 列:一列(数据元素)包含了相同类型的数据,例如邮政编码的数据。

4. 行:一行(=元组,或记录)是一组相关的数据,例如-一条用户订阅的数据。

5. 冗余:存储两倍数据,冗余降低了性能,但提高了数据的安全性。

6. 主键:主键是唯一的。一个数据表中只能包含一个主键。你可以使用主键来查询数据。

7. 外键:外键用于关联两个表。

8. 复合键:复合键(组合键)将多个列作为一个索引键,一般用于复合索引。

9. 索引:使用索引可快速访问数据库表中的特定信息。索引是对数据库表中一列或多列的
   值进行排序的一种结构。类似于书籍的目录。

10. 参照完整性:参照的完整性要求关系中不允许引用不存在在的实体。与实体完整性是关系
    模型必须满足的完整性约束条件,目的是保证数据的一致性""""

#### MySQL数据库

MySQL为关系型数据库(Relational Database Management System),这种所谓的"关系型"可以
理解为"表格"的概念,一个关系型数据库由一个或数个表格组成如图所示的一个表格:

![mysql](Z:\0voice\Linux入门环境编程\5. 数据库mysql项目实战\mysql.png)

- 

#### MySQL的安装:

```
sudo apt-get install mysql-server-5.6
```

出现报错：

```
The following packages have unmet dependencies:
 mysql-server-5.6 : Depends: libdbi-perl but it is not going to be installed
                    Depends: mysql-client-5.6 (>= 5.6.33-0ubuntu0.14.04.1) but it is not going to be installed
```

不知道怎么解决，问：

king老师，我遇到一个环境安装问题（安装MySQL数据库失败），想请教您，我使用：`$sudo apt-get install mysql-server-5.6` 命令在`ubuntu1y.04`上安装报错缺少相关依赖：`libdbi-perysql`和 `mysql-client-5.6 (>= 5.6.33-0ubuntu0.14.04.1)`，我目前已经尝试过的方法有：1. 把source.list源分别换成群里提供的阿里源和原始的源,并`$sudo apt update`更新；2. rpm -qa检查已知从未安装过mysql库； 3. mysql官网发现mysql-server-5.6支持的系统最高只到ubuntu14.04；

以上方法目前都不行，请king老师指导一下

![image-20250415101507223](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250415101507223.png)

![image-20250415102700379](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250415102700379.png)

**解决方法：**

1. 首先 `lsb_release -a`查看系统codename

2. 打开 `vim /etc/apt/sources.list`，发现目前版本为`trusty`（14.04）版本，与目前系统的 `xenial 16.04`版本不对应，全部改成xenial版本即可

3. ```
   sudo apt-get update
   sudo sudo apt-get install mysql-server
   ```

   下载成功

4. 另外vim中批量替换代码：

   ```
   :%s/old_text/new_text/g
   ```

   

#### MySQL操作命令：

##### MySQL远程连接

```
mysql -u root -p
```

- 如果登录失败，使用sudo用户登陆即可

- 遇到错误：`error:10061`;解决方法：在`/etc/mysql/my.cnf`中修改回环地址`bing address：127.0.0.1`为`0.0.0.0`

  - 实际情况中，bing address不在`/etc/mysql/my.cnf`而在`/etc/mysql/mysql.conf.d/mysqld.cnf`中，
  - 记得重启：`sudo systemctl restart mysql`或者 `sudo /etc/init.d/mysql restart`

- 遇到错误：`服务器限制root远程登录`;解决方法：重新创建一个user

  ```
  use mysql;
  create user 'admin'@'%' identified by '12345678';
  ```

  - 企业生产中不要更改`root`用户的内容
  - 作用是创建一个名为 admin，并且可以从任意主机（% 表示任意主机）连接到数据库的用户，其密码设置为 123456 

- 遇到错误：`SSL connection error` 问题，需要关闭MySQL服务端的ssh连接，**这个没有解决**，因为MySQL8.0默认使用ssh，以后再进行学习。

- 遇到错误：admin用户没有权限；解决方案：让root用户赋予admin用户权限：`grant select on mysql.* to 'admin'@'%';`或者直接所有权限：`grant all privileges on *.* to 'admin'@'%'`(不建议); `SHOW GRANTS FOR 'admin'@'%';`查看权限

  - 用 `create user 'admin'@'%' identified by 'cv';`创建admin用户，任意地址都可以登录该用户
  - `Workbench` 进入数据库，输入命令 `show databases;use mysql;`，发现用户没有权限使用数据表
  - 使用`grant select on mysql.* to 'admin'@'%'`获取select权限
  - 再次执行 `use mysql;show tables;select user,host from user;`语句发现权限已经获取，但是不能创建用户： `create user 'test'@'%' identified by 'cv'`,没有获取创建的权限
  - 申请所有权限：`grant all privileges on *.* to 'admin'@'%'`,并刷新权限 `flush privileges` 此时已经可以创建用户

- `grant `命令用法

  ```
  GRANT privileges 
  ON database.table 
  TO user [IDENTIFIED BY 'password']
  [WITH GRANT OPTION];
  ```

  - privileges：这是你要授予的权限，它可以是单个权限，也可以是多个权限组成的列表。常见的权限有 SELECT、INSERT、UPDATE、DELETE、ALL PRIVILEGES（所有权限）等。
  - database.table：指定权限适用的数据库和表。可以使用 * 作为通配符，例如 mydb.* 代表 mydb 数据库里的所有表，*.* 则代表所有数据库的所有表。
  - user：是要授予权限的用户，格式为 'username'@'host'。username 是用户名，host 是允许用户登录的主机，可以是具体的 IP 地址、域名，也可以使用 % 通配符（代表任何主机）。
  - IDENTIFIED BY 'password'：若用户还未创建，可使用此选项为用户设置密码。
  - WITH GRANT OPTION：赋予用户将自己拥有的权限再授予其他用户的能力。

##### MySQL操作命令

- 操作命令不区分大小写

- 创建数据库：`CREATE DATABASE 数据库名`

- 创建数据表：`CREATE TABLE 数据表名`

- 查询数据：

  ```
  SELECT column_name,column_name
  FROM table_name
  [WHERE Clause]
  [LIMIT N][ OFFSET M]
  ```

  - 可以用`*`来代替其他字段，会返回表的所有字段数据

  - `WHERE`字句用来选取条件，相当于`if`

    例如：

    ```
    select Host,User from user where User='root';
    ```

- `UPDATE table_name SET field1=new-value1, field2=neew-value2 [WHERE Clause]；`修改数据

- `show databases;`查看数据库(注意s)

- `use mysql;`使用mysql数据库

- `show tables;`查看数据表

- `select * from user;`查看数据表内容

- 删除数据库：`DROP`

- 创建数据表：

  ```
  CREATE TABLE TBL_USER( #
  U_ID INT PRIMARY KEY AUTO_INCREMENT,
  U_NAME VARCHAR(32),
  GENGDER VARCHAR(8)
  );
  ```

  - U_ID 是表中的一个列名。

  - INT 表明该列的数据类型为整数。

  - PRIMARY KEY 说明 U_ID 列是这个表的主键，主键能够唯一标识表中的每一行数据，且不能包含重复值与 NULL 值。

  - AUTO_INCREMENT 表示该列的值会自动递增，每次插入新行时，数据库会自动为 U_ID 赋予一个比当前最大值大 1 的新值。
  - U_NAME 是表中的另一个列名。VARCHAR(32) 意味着该列的数据类型为可变长度的字符串，最多能存储 32 个字符。

- 在数据表中插入数据：

  ```
  INSERT INTO table_name(field1, field2, ...,fieldN) VALUES(value1, value2, ...,valueN);
  ```

- 在数据表中删除数据：

  ```
  DELETE FROM table_name WHERE filed_name='fileds';
  ```

  报错：

  > Error Code: 1175. You are using safe update mode and you tried to update a table without a WHERE that uses a KEY column.  To disable safe mode, toggle the option in Preferences -> SQL Editor and reconnect.

​	原因，选择U_NAME作为key删除可能会导致多行删除，不是安全模式所以不能删除，所以设置成安全模式

```
SET SQL_SAFE_UPDATES=0;
DELETE FROM TBL_USER WHERE U_NAME='hsy';
SET SQL_SAFE_UPDATES=1;
```

- 存储过程：`PROC_DELETE_USER`

  ```
  DELIMITER $$
  CREATE PROCEDURE PROC_DELETE_USER(IN UNAME VARCHAR(32))
  BEGIN
      SET SQL_SAFE_UPDATES=0;
      DELETE FROM TBL_USER WHERE U_NAME=UNAME;
      SET SQL_SAFE_UPDATES=1;
  END$$
  DELIMITER ;
  ```

  - 在 MySQL 里，默认情况下会把分号 ; 当作 SQL 语句的结束符。当你输入一条 SQL 语句并以 ; 结尾时，MySQL 就会认为这条语句已经完整，接着开始执行它。
  - 存储过程是一组预先编译好的 SQL 语句集合，它内部通常包含多条 SQL 语句，而且每条语句都以 `;` 结尾。要是直接使用默认的 `;` 作为结束符，在创建存储过程时，MySQL 会在遇到第一个 `;` 时就尝试执行前面的部分，而不会把整个存储过程当作一个完整的单元来处理，这样就会导致存储过程创建失败。\
  - `DELIMITER`会把 `$$` 当作语句结束符
  - 通常在创建完存储过程之后，需要使用 `DELIMITER ;` 把结束符恢复为默认的 `;`，这样后续的 SQL 语句就能正常使用 `;` 作为结束符了

  调用存储过程：

  ```
  CALL PROC_DELETE_USER('ikun');
  ```


- 这是一条在 MySQL 数据库中用于修改表结构的 SQL 语句，它的作用是向名为 `TBL_USER` 的表中添加一个名为 `U_IMAGE` 的列，该列的数据类型为 `BLOB`（二进制大对象）。以下是对这条语句的详细解释：;

```
ALTER TABLE TBL_USER ADD U_IMAGE BLOB;
```





### 2 MySQL数据库编程连接与插入数据

#### 1 数据库服务器创建数据库表

```
DROP DATABASE HSY_DB;

CREATE DATABASE HSY_DB;

show databases;

USE HSY_DB; # 使用数据库

CREATE TABLE TBL_USER ( # 创建表
U_ID INT PRIMARY KEY AUTO_INCREMENT,
U_NAME VARCHAR(32),
U_GENDER VARCHAR(8)
);

SHOW TABLES; # 显示表
```

#### 2 在NS(node server)服务器上连接MySQL服务器

##### 安装MySQL客户端开发工具：

```
sudo apt-get install libmysqlclient-dev
```

注：若要在 Win10 的 C 语言项目里添加 mysql.h 的 include 路径，需对 c_cpp_properties.json 文件进行修改

具体步骤如下：

1. 打开 VSCode：打开你的 C 语言项目文件夹。
2. 创建或打开 c_cpp_properties.json 文件：按下 Ctrl + Shift + P，输入 C/C++: Edit Configurations (JSON) 并回车，这会打开或创建 c_cpp_properties.json 文件。
3. 添加 MySQL include 路径：在 includePath 数组里添加 MySQL 的 include 路径 `"C:/Program Files/MySQL/MySQL Server 5.7/include"`
4. 保存文件：保存 c_cpp_properties.json 文件。

##### 使用MySQL实现CRUD:

###### mysql的声明连接初始化：

```
#include <stdio.h>

#include <mysql.h>

#define HSY_DB_SERVER_IP   "192.168.21.129"
#define HSY_DB_SERVER_PORT 3306

#define HSY_DB_USERNAME    "admin"
#define HSY_DB_PASSWORD    "cv"

#define HSY_DB_DEFAULTDB   "HSY_DB"
// C U R D

int main(){
    // 声明mysql
    MYSQL mysql; 

    // 初始化mysql
    if (NULL == mysql_init(&mysql)){
        printf("mysql_init : %s\n", mysql_error(&mysql));
        return -1;
    }

    // 连接mysql
    mysql_real_connect(&mysql, HSY_DB_SERVER_IP, HSY_DB_USERNAME,
         HSY_DB_PASSWORD, HSY_DB_DEFAULTDB, HSY_DB_SERVER_PORT, NULL, 0);
    //...
    
    return 0;
}
```

- 返回值：对 `mysql_real_connect` 的返回值加以检查，若为 NULL，则表明连接失败，利用 mysql_error 函数获取错误信息；若不为 NULL，则表示连接成功。

###### 插入数据库信息：

```
// insert
    if(0 ！= mysql_real_query(&mysql, SQL_INSERT_TBL_USER, strlen(SQL_INSERT_TBL_USER))){
        printf("mysql_insert: %s\n", mysql_error(&mysql));
        return -3;
    }
```

编译时出现问题，只有手动链接mysql头文件时才能包含mysql头文件：

```
sudo gcc 1_Mysql.c -o 1_Mysql -lmysqlclient -I/usr/include/mysql
```

于是



###### 查询数据库信息：

```
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
```

- `MYSQL_RES *res = mysql_store_result(handle);`获取结果
- `mysql_num_rows` 输出数据行数 `mysql_num_fields` 输出数据列数
- `mysql_fetch_row`获取每行的信息，如果多列以数组返回
- `mysql_free_result(res);`释放缓存
- `mysql_close(&mysql);`关闭数据库

###### 删除数据库信息：

```
SET SQL_SAFE_UPDATES=0;
DELETE FROM TBL_USER WHERE U_NAME='hsy';
SET SQL_SAFE_UPDATES=1;
```

在代码里如何把三条代码写到一起，定义一个存储过程：

```
DELIMITER $$
CREATE PROCEDURE PROC_DELETE_USER(IN UNAME VARCHAR(32))
BEGIN
    SET SQL_SAFE_UPDATES=0;
    DELETE FROM TBL_USER WHERE U_NAME=UNAME;
    SET SQL_SAFE_UPDATES=1;
END$$
DELIMITER ;
```

- 在 MySQL 里，默认情况下会把分号 ; 当作 SQL 语句的结束符。当你输入一条 SQL 语句并以 ; 结尾时，MySQL 就会认为这条语句已经完整，接着开始执行它。
- 存储过程是一组预先编译好的 SQL 语句集合，它内部通常包含多条 SQL 语句，而且每条语句都以 `;` 结尾。要是直接使用默认的 `;` 作为结束符，在创建存储过程时，MySQL 会在遇到第一个 `;` 时就尝试执行前面的部分，而不会把整个存储过程当作一个完整的单元来处理，这样就会导致存储过程创建失败。\
- `DELIMITER`会把 `$$` 当作语句结束符
- 通常在创建完存储过程之后，需要使用 `DELIMITER ;` 把结束符恢复为默认的 `;`，这样后续的 SQL 语句就能正常使用 `;` 作为结束符了

调用存储过程：

```
CALL PROC_DELETE_USER('ikun');
```

#### 使用MySQL实现图片存储

磁盘读取图片：

```
int read_image(char *filename, char *buffer){
	if (filename == NULL || )
}
```

- `fopen(filename ,"rb")` 打开文件
- `fseek(fp, 0, SEEK_END)` 用于移动文件指针的位置，到文件末尾；
- `ftell` 读取文件指针的位置，正好是文件的大小；
- `fread(buffer, 1, length, fp);`表示从文件指针 fp 所指向的文件中读取 length 个字节的数据，每个数据项的大小为 1 字节，将读取的数据存储到 buffer 指向的内存区域。函数返回实际读取的数据项数量。

图片写入磁盘：

```
int write_image(char *filename, char *buffer, int length)
```

buffer存入数据库：

```
int mysql_write(MYSQL *handle, char *buffer, int length){

}
```

- `mysql_stmt_init(handle)`：初始化一个 `MYSQL_STMT` 对象，用于后续的预处理语句操作。
- `mysql_stmt_prepare(stmt, SQL_INSERT_IMG_USER, strlen(SQL_INSERT_IMG_USER))`：准备一条 SQL 插入语句
- `MYSQL_BIND` 结构体用于绑定参数。这里将参数类型设置为 `MYSQL_TYPE_LONG_BLOB`，表示要插入的是大二进制对象。
- `mysql_stmt_bind_param(stmt, &param)`：将参数绑定到预处理语句上。若绑定失败，函数会输出错误信息并返回 `-3`
- `mysql_stmt_send_long_data(stmt, 0, buffer, length)`：将二进制数据发送到 MySQL 服务器。`0` 表示参数的索引（这里只有一个参数）。若发送失败，函数会输出错误信息并返回 `-4
- `mysql_stmt_execute(stmt)`：执行预处理语句，将数据插入到数据库中。若执行失败，函数会输出错误信息并返回 -5
- `mysql_stmt_close(stmt)`：关闭预处理语句，释放相关资源。若关闭失败，函数会输出错误信息并返回 -6

- 总结：准备数据库句柄，stmt初始化，bind参数设置（数据类型，数据长度，数据buffer和is_null），参数注入stmt，**stmt发送send**，stmt执行，stmt关闭。

从数据库读图片：

```

```

- 
- 总结：准备数据库句柄，stmt初始化，bind参数设置（数据类型，数据总长度），参数注入，stmt执行，stmt接受store_result，stmt通过循环获取列把数据读入buffer中，stmt关闭

在main函数中写好测试代码：

```
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
```

- 步骤：定义buffer，读取图片，发送到mysql，mysql读取，写入图片



注意：

- 写入文件时，wb+ 整体表示以二进制可读写模式打开文件，如果文件不存在就创建一个新的二进制文件，如果文件存在则先清空文件内容（另外需要sudo权限写入图片）

全部代码：

```
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
```

总结：

- 
- 作业题：自己实现一个mysql数据库的连接池

### 3 自己实现一个mysql数据库的连接池

```
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
#define SQL_SELECT_IMG_USER "SELECT U_IMAGE FROM TBL_USER WHERE U_NAME='ikun';"
#define FILE_IMAGE_LENGTH (64*1024)
#define CONNECTION_POOL_SIZE 5

// 连接池结构体
typedef struct {
    MYSQL *connections[CONNECTION_POOL_SIZE];
    int used[CONNECTION_POOL_SIZE];
    pthread_mutex_t lock;
} ConnectionPool;

// 初始化连接池
void init_connection_pool(ConnectionPool *pool) {
    pthread_mutex_init(&pool->lock, NULL);
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
            pool->used[i] = 0;
        }
    }
}

// 从连接池获取连接
MYSQL *get_connection(ConnectionPool *pool) {
    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        if (!pool->used[i]) {
            pool->used[i] = 1;
            pthread_mutex_unlock(&pool->lock);
            return pool->connections[i];
        }
    }
    pthread_mutex_unlock(&pool->lock);
    return NULL;
}

// 归还连接到连接池
void release_connection(ConnectionPool *pool, MYSQL *conn) {
    pthread_mutex_lock(&pool->lock);
    for (int i = 0; i < CONNECTION_POOL_SIZE; i++) {
        if (pool->connections[i] == conn) {
            pool->used[i] = 0;
            break;
        }
    }
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
// 线程执行的函数
void* thread_function(void* arg) {
    ConnectionPool *pool = (ConnectionPool*)arg;
    MYSQL *conn = get_connection(pool);
    if (conn != NULL) {
        if (0 != mysql_real_query(conn, SQL_SELECT_TBL_USER, strlen(SQL_SELECT_TBL_USER))) {
            printf("mysql_real_query: %s\n", mysql_error(conn));
        }
        release_connection(pool, conn);
    }
    return NULL;
}
// 其他函数保持不变
// ...

int main() {
    ConnectionPool pool;
    init_connection_pool(&pool);

    // 示例：获取连接并执行查询
    MYSQL *conn = get_connection(&pool);
    if (conn != NULL) {
        if (0 != mysql_real_query(conn, SQL_SELECT_TBL_USER, strlen(SQL_SELECT_TBL_USER))) {
            printf("mysql_real_query: %s\n", mysql_error(conn));
        }
        release_connection(&pool, conn);
    }

    destroy_connection_pool(&pool);
    return 0;
}
```

以上为关键代码