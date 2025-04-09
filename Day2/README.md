## 【Linux入门环境编程】Linux文件操作（统计单词数量）+结构体应用（实现通讯录）

### 1 Linux文件操作——项目：统计文件单词数量

#### 项目目的：

- 如何统计一个文件中所有的单词数量，或者分别统计文件中每一个单词数量

#### 背景知识：

- 一篇小说可能包含的非字符符号有

  / `\n` `\t` `\"` `\'` `+` `,` `;` `.`

-  状态机

  状态机有三个状态INIT/OUT/IN，状态机改变状态有两种路径，当输入为分隔符时，改变状态为OUT，否则当状态为OUT且输入不为分隔符是，改变状态为IN；在统计单词数量的代码中，OUT 代表不在单词内，IN 代表在单词内，这两个状态及其转换规则让代码逻辑变得直观易懂。

![image-20250408102727782](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250408102727782.png)

- **统计一个文件中所有的单词数量**代码实现：

  ```
  #include<stdio.h>
  #include<stdlib.h>
  #include<string.h>
  
  
  #define OUT 0
  #define IN 1
  #define INIT OUT
  
  int split(char c){
      if(c == ('/')||('\n')||('\t')||('\"')||('+')||(',')||
      ('.')||('?')||('!')||('(')||(')')||('-')||(':')||(';')){
          return 1;
      }
      else 
          return 0;
  }
  
  int count(char* file_addr){
      int status = INIT;
      int word = 0;
      char c; 
      FILE* fp = fopen(file_addr, "r");
      if (fp == NULL) return -1;
      while((c = fgetc(fp)) != EOF){
          if(split(c)){
              status = OUT;
          }
          else if(status == OUT){
              status = IN;
              word++;
          }
      }
      return word;
      
  }
  
  int main(int argc, char* argv[]){
      if(argc < 2){
          printf("Please insert file address!\n");
      }
      else{
          printf("the word number of file is %d\n", count(argv[1]));
      }
  
  }
  ```

  - 读文件用fopen(filename, "r")
  - 如果文件不存在要跳出
  - 按照字符读文件用fgetc(FILE*),可以用while循环按顺序读
  - 判断文件结束用!=EOF标识符
  - main函数常用(int argc, char* argv[]),一般用argc判断输入参数是否足够,不够的话报错
  - argv是个数组,取用参数的话要用argv[1]
  - 注意==和=号不要打错了,容易混淆

此时输出一直等于0,debug后发现split的if判断写错了,要判断c是否与这些标识符相等,而不是把这些表示符号求或再判断,改正代码:

```
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


#define OUT 0
#define IN 1
#define INIT OUT

int split(char c){
    if((c == '/')||(c == '\n')||(c == '\t')||
    (c == '\"')||(c == '+')||(c == ',')||
    (c == '.')||(c == '?')||(c == '!')||
    (c == '(')||(c == ')')||(c == '-')||
    (c == ':')||(c == ';')){
        return 1;
    }
    else 
        return 0;
}

int count(char* file_addr){
    int status = INIT;
    int word = 0;
    char c; 
    FILE* fp = fopen(file_addr, "r");
    if (fp == NULL) return -1;
    while((c = fgetc(fp)) != EOF){
        if(split(c)){
            status = OUT;
        }
        else if(status == OUT){
            status = IN;
            word++;
        }
    }
    return word;
    
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Please insert file address!\n");
    }
    else{
        printf("the word number of file is %d\n", count(argv[1]));
    }

}
```

结果输出为:

```
./1_count a.txt
the word number of file is 9
```

- **分别统计文件中每一个单词数量**代码实现:

  - 定义一个哈希表unordered_map,存储单词名称 出现次数;

  - 判断单词结束:如果遇到分隔符且从状态IN变为OUT,将该词语buffer全部小写并插入到哈希表中去,并且词汇buffer计零;
  - 判断单词开始:如果没有遇到分隔符且从状态OUT变为IN,词语加一,buffer增加改字符;

  ```
  #include<stdio.h>
  #include<stdlib.h>
  #include<string.h>
  
  //麻了，直接用C++里的哈希表了
  #include<unordered_map>
  #include<iostream>
  using namespace std;
  
  
  #define OUT 0
  #define IN 1
  #define INIT OUT
  
  int split(char c){
      if((c == '/')||(c == '\n')||(c == '\t')||
      (c == '\"')||(c == '+')||(c == ',')||
      (c == '.')||(c == '?')||(c == '!')||
      (c == '(')||(c == ')')||(c == '-')||
      (c == ':')||(c == ';')||(c == ' ')){
          return 1;
      }
      else 
          return 0;
  }
  
  int count(char* file_addr){
      unordered_map<string, int> wordCount;
      int status = INIT;
      int word = 0;
      string buffer;
      char c; 
      FILE* fp = fopen(file_addr, "r");
      if (fp == NULL) return -1;
      while((c = fgetc(fp)) != EOF){
          if(split(c)){
              if(status == IN){
                  for (char& ch : buffer){
                      ch = tolower(ch);
                  }
                  wordCount[buffer]++;
                  buffer.clear();
              }
              status = OUT;
          }
          else{
              if (status == OUT){
                  status = IN;
                  word++;
              }
              buffer += c;
          }
      }
      if(status == IN){
          for (char& ch : buffer){
              ch = tolower(ch);
          }
          wordCount[buffer]++;
          buffer.clear();
      }
      for (const auto& pair : wordCount) {
          std::cout << pair.first << ": " << pair.second << std::endl;
      }
  
      return word;
      
  }
  
  int main(int argc, char* argv[]){
      if(argc < 2){
          printf("Please insert file address!\n");
      }
      else{
          printf("word: %d\n", count(argv[1]));
      }
  
  }
  ```

### 2 Linux结构体/双向链表运用——项目：实现通讯录

#### 思路梳理

##### 需求分析

1.添加一个人员

2.打印显示所有人员

3.删除一个人员

4.查找一个人员

5.保存文件

6.加载文件

##### 技术选型

- 链表实现人员存储
- 结构体实现文件存储(姓名,电话,年龄,地址)
- 注意将数据结构的操作与业务分离

##### 方案设计

- 第一阶段:(底层)实现链表的增删改查,以及人员结构体的读和写
- 第二阶段:(接口层)人员的增删查遍历,文件的解包和打包
- 第三阶段:(业务层)

#### 代码实现

##### 链表的增删改查

1. 链表新增逻辑:
   - item接在list表前,next指向list
   - 如果list非空,还要把原list的前指针指向item
   - 用item替代list
2. 链表删除逻辑:
   - 如果item不是第一个,需要把前一个的后指针指向下一个
   - 如果item不是最后一个,需要把后一个的前指针指向上一个
   - 如果item是list第一个,只需要把list指向后一个
   - 删除item的前后指针

```c
#define LIST_INSERT(item, list) do {       \
    (item)->prev = NULL;                   \
    (item)->next = list;                   \
    if ((list) != NULL) (list)->prev = (item);\
    (list) = (item);                          \
}while(0)

#define LIST_REMOVE(item, list) do {                           \
    if((item)->prev != NULL) (item)->prev->next = (item)->next;\
    if((item)->next != NULL) (item)->next->prev = (item)->prev;\
    if((item) == (list)) (list) = (item)->next;                \
    (item)->next = (item)->prev = NULL;                        \
}while(0)
```

- define函数后面要使用do...while(0)结构的宏定义,这样可以避免在函数中出现if找错else等奇怪的bug
- 把printf定义为INFO,这样如果上线的时候,直接把INFO后的定义为空,就可以删除所有打印信息
- string.h包含了memset以及strcmp函数
- 链表操作一定要避免**指向一块已经回收的内存**
- 建议把宏定义的参数使用时用括号括一下
- 宏定义的参数就是替换，不需要制定类型
- 宏定义换行时需要加反斜杠`\`

##### 人员的结构体定义

```c
struct person{
    char name[MAX_NAME];
    char phone[MAX_PHONE_NUMBER];

    struct person* next;
    struct person* prev;
};

struct contacts{
    struct person* people;
    int count;
};
```

- 结构体后要加`;`
- name和phone用控制上限的数组,而不是没有上限的数组指针
- 结构体中有两个以自己的结构体为类型的前后指针
- 结构体内变量不需要初始化,数组用宏定义固定大小即可

##### 人员的增删改查

```
int person_add(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    LIST_INSERT(ps, *ppeople);
    return 0;
}

int person_delete(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    if(ppeople == NULL) return -2;
    LIST_REMOVE(ps, *ppeople);
    return 0;
}

struct person* person_search(struct person *people, const char *name){
    //遍历链表查name
    struct person* item = NULL;
    for(item = people; item != NULL; item = item->next){
        if(!strcmp(item->name, name)) break;
    };
    return item;
}

int person_traversal(struct person *people){
    struct person* item = NULL;
    for(item = people; item != NULL; item = item->next){
        INFO("name: %s, phone: %s\n", item->name, item->phone);
    };
    return 0;
}
```

- 当需要操作指针代表的数组自己时,请使用**双指针**
- ppeople：代表的是指向 struct person 类型指针的指针，用于检查这个指针本身是否为 NULL。 
- *ppeople：是对 ppeople 解引用后得到的 struct person 类型的指针，作为参数传递给 LIST_INSERT 和 LIST_REMOVE 函数。
- 搜索功能用一个临时结构体的遍历实现,
- strcmp如果相等返回0,否则返回正负值(根据ASCII码大小)

##### 业务逻辑的实现(外部增删查遍历打印)

1. 插入逻辑
   - 动态分配一个结构体内存p
   - 输入名字和电话
   - 插入链表
   - 通讯录更新

```
int insert_entry(struct contacts* cts){
    //动态分配一个结构体内存p
    if(cts == NULL) return -1;
    struct person* p = (struct person*)malloc(sizeof(struct person));
    if(p == NULL) return -2;
    memcpy(p, 0, sizeof(struct person));

	//name
	INFO("Insert name:\n");
    scanf("%s", p->name);

	//phone,
	INFO("Insert phone:\n");
    scanf("%s", p->phone);

	//add people
    if(0 != person_add(&cts->people, p)){
        free(p);
        return -3;
    }

    cts->count++;
    INFO("Insert SUCCESS!\n");
    return 0;

}

int print_entry(struct contacts* cts){
	if (cts == NULL) return -1;

	//cts->people
    person_traversal(cts);
}

int delete_entry(struct contacts* cts){
	//动态分配一个结构体内存p
    if(cts == NULL) return -1;

	//name
    char name[MAX_NAME];
	INFO("Delete name:\n");
    scanf("%s", name);

	//person_search
	struct person *p = person_search(cts->people, name);
    if (p == NULL){
        INFO("Person NOT Found!");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s", p->name, p->phone);
	//delete
    person_delete(&cts->people, p);
    free(p);

    cts->count--;
    INFO("Success Delete!");
    return 0;
	
}

int search_entry(struct contacts* cts){
	//动态分配一个结构体内存p
    if(cts == NULL) return -1;

    //input_name
	char name[MAX_NAME];
	INFO("Search name:\n");
    scanf("%s", name);

	//person_search
    struct person *p = person_search(cts->people, name);
    if (p == NULL){
        INFO("Person NOT Found!");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s", p->name, p->phone);

    return 0;
}
```

- 如果加载失败,注意释放malloc申请的空间
- malloc申请的空间一定要记得memset重置为0
- cts->people 是一个链表的头指针。当向链表插入新节点时，可能会改变头指针的值。例如，若链表原本为空，插入第一个节点后，头指针就需要指向这个新节点。为了能在 person_insert 函数内部修改 cts->people 这个指针本身的值，就必须传递它的地址（即指向该指针的指针）。

##### 操作类型枚举

```
enum{
    OPER_INSERT = 1,
    OPER_PRINT,
	OPER_DELETE,
	OPER_SEARCH,
	OPER_SAVE,
	OPER_LOAD
};
```



##### 主函数实现

```
int main(){
	//初始化
	struct contacts *cts = (struct contacts*)malloc(sizeof(struct contacts));
    if (cts == NULL) return -1;
    memcpy(cts, 0, sizeof(struct contacts));

    
	//业务循环
    while(1){
        menu_info();
        
        int insert = 0;
        scanf("%d", insert);
        switch(insert){
            case OPER_INSERT:
				insert_entry(cts);
				break;

			case OPER_PRINT:
				print_entry(cts);
				break;

			case OPER_DELETE:
				delete_entry(cts);
				break;

			case OPER_SEARCH:
				search_entry(cts);
				break;

			case OPER_SAVE:
				save_entry(cts);
				break;

			case OPER_LOAD:
				load_entry(cts);
				break;

			default:
				goto exit;

        }
    }
    goto exit;
	//释放malloc分配的内存
exit:
    free(cts);
    return 0;
}
```

- `goto` 语句和 `exit` 标签是一起使用的，`goto` 语句用于无条件跳转到指定的标签处，`exit` 在这里是一个用户自定义的标签。
- 虽然 `goto` 语句在某些情况下可以简化代码逻辑，但过度使用可能会使代码变得难以理解和维护，因此在实际编程中应谨慎使用。

##### 添加提示信息

```
void menu_info(void) {

	INFO("\n\n********************************************************\n");
	INFO("***** 1. Add Person\t\t2. Print People ********\n");
	INFO("***** 3. Del Person\t\t4. Search Person *******\n");
	INFO("***** 5. Save People\t\t6. Load People *********\n");
	INFO("***** Other Key for Exiting Program ********************\n");
	INFO("********************************************************\n\n");

}
```

##### 文件保存和加载

- fopen:打开一个文件
- fprintf:按照指定格式写入文件
- fflush:从内存中同步到磁盘中
- fclose:关闭一个打开的文件
- feof:是否读到文件的结尾
- fgets:读取一行数据

```
int save_file(struct person *people, const char *filename){
	//打开文件
	FILE *fp = fopne(filename, "w");
    if (fp == NULL) return -1;
	
    //循环文件读取
    struct person* temp = NULL;
    for (temp = people; temp != NULL; temp = temp->next){
        fprintf(fp, "name: %s, phone: %s", temp->name, temp->phone);
        fflush(fp);
    }
}

int parser_token(char *buffer, int length, char *name, char *phone){
	//检查缓冲区指针，和buffeer长度
	if (buffer == NULL) return -1;
    if (length < MIN_TOKEN_LENGTH) return -2;

	//name: qiuxiang, telephone: 98765678123
	//使用两层状态机解析文件
    int i = 0, j = 0, status = 0;
    for(i = 0; buffer[i] != ','; i++){
        if(buffer[i] == ' '){
            status = 1;
        }else if(status == 1){
            name[j++] = buffer[i];
        }
    }

    j = 0, status = 0;
    for(; i < length; i++){
        if(buffer[i] == ' '){
            status = 1;
        }else if(status == 1){
            phone[j++] = buffer[i];
        }
    }

    INFO("file token : %s --> %s\n", name, phone);

	return 0;
}

int load_file(struct person **ppeople, int *count, const char *filename){
	//打开文件
	FILE *fp = fopne(filename, "r");
    if (fp == NULL) return -1;
	
    //循环读取文件
    while(!feof(fp)){
        char buffer[BUFFER_LENGTH] = {0};
		fgets(buffer, BUFFER_LENGTH, fp);
		int length = strlen(buffer);
		INFO("length : %d\n", length);

        // name: qiuxiang,telephone: 98765678123
		char name[MAX_NAME] = {0};
		char phone[MAX_PHONE_NUMBER] = {0};

		if (0 != parser_token(buffer, length, name, phone)) {
			continue;
		}

        struct person *p = (struct person*)malloc(sizeof(struct person));
		if (p == NULL) return -2;

		memcpy(p->name, name, MAX_NAME);
		memcpy(p->phone, phone, MAX_PHONE_NUMBER);

		person_insert(ppeople, p);
		
		(*count) ++;
    }
    
}
```

- 通常情况下，程序的输出会先被存储在缓冲区里，等缓冲区满了或者遇到换行符时才会真正输出。使用 fflush 能够强制将缓冲区中的内容输出。使用 fflush(stdin) 可以尝试清空输入缓冲区。
- 在函数内部若要修改传入的指针变量，就需要用到二级指针。因为在 C 和 C++ 里，函数参数是按值传递的，直接传递指针时，函数接收的是指针的副本，修改该副本不会影响原始指针。
- 数字尽量用宏定义。
- 

##### 文件业务逻辑

```
int save_entry(struct contacts *cts) {

	if (cts == NULL) return -1;

	INFO("Please Input Save Filename :\n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);

	save_file(cts->people, filename);
	
}

int load_entry(struct contacts *cts) {
	if (cts == NULL) return -1;

	INFO("Please Input Load Filename :\n");
	char filename[NAME_LENGTH] = {0};
	scanf("%s", filename);

	load_file(&cts->people, &cts->count, filename);
}
```

#### 完整代码:

```
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_PHONE_NUMBER 32
#define MAX_NAME    16
#define MIN_TOKEN_LENGTH 5
#define BUFFER_LENGTH		128

#define INFO printf

#define LIST_INSERT(item, list) do {       \
    (item)->prev = NULL;                   \
    (item)->next = list;                   \
    if ((list) != NULL) (list)->prev = (item);\
    (list) = (item);                          \
}while(0)

#define LIST_REMOVE(item, list) do {                           \
    if((item)->prev != NULL) (item)->prev->next = (item)->next;\
    if((item)->next != NULL) (item)->next->prev = (item)->prev;\
    if((item) == (list)) (list) = (item)->next;                \
    (item)->next = (item)->prev = NULL;                        \
}while(0)

enum{
    OPER_INSERT = 1,
    OPER_PRINT,
	OPER_DELETE,
	OPER_SEARCH,
	OPER_SAVE,
	OPER_LOAD
};

struct person{
    char name[MAX_NAME];
    char phone[MAX_PHONE_NUMBER];

    struct person* next;
    struct person* prev;
};

struct contacts{
    struct person* people;
    int count;
};

int person_insert(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    LIST_INSERT(ps, *ppeople);
    return 0;
}

int person_delete(struct person **ppeople, struct person *ps){
    if(ps == NULL) return -1;
    if(ppeople == NULL) return -2;
    LIST_REMOVE(ps, *ppeople);
    return 0;
}

struct person* person_search(struct person *people, const char *name){
    //遍历链表查name
    struct person* item = NULL;
    for(item = people; item != NULL; item = item->next){
        if(!strcmp(item->name, name)) break;
    };
    return item;
}

int person_traversal(struct person *people){
    struct person* item = NULL;
    for(item = people; item != NULL; item = item->next){
        INFO("name: %s, phone: %s\n", item->name, item->phone);
    };
    return 0;
}

int save_file(struct person *people, const char *filename){
	//打开文件
	FILE *fp = fopen(filename, "w");
    if (fp == NULL) return -1;
	
    //循环文件读取
    struct person* temp = NULL;
    for (temp = people; temp != NULL; temp = temp->next){
        fprintf(fp, "name: %s, phone: %s", temp->name, temp->phone);
        fflush(fp);
    }
}

int parser_token(char *buffer, int length, char *name, char *phone){
	//检查缓冲区指针，和buffeer长度
	if (buffer == NULL) return -1;
    if (length < MIN_TOKEN_LENGTH) return -2;

	//name: qiuxiang, telephone: 98765678123
	//使用两层状态机解析文件
    int i = 0, j = 0, status = 0;
    for(i = 0; buffer[i] != ','; i++){
        if(buffer[i] == ' '){
            status = 1;
        }else if(status == 1){
            name[j++] = buffer[i];
        }
    }

    j = 0, status = 0;
    for(; i < length; i++){
        if(buffer[i] == ' '){
            status = 1;
        }else if(status == 1){
            phone[j++] = buffer[i];
        }
    }

    INFO("file token : %s --> %s\n", name, phone);

	return 0;
}

int load_file(struct person **ppeople, int *count, const char *filename){
	//打开文件
	FILE *fp = fopen(filename, "r");
    if (fp == NULL) return -1;
	
    //循环读取文件
    while(!feof(fp)){
        char buffer[BUFFER_LENGTH] = {0};
		fgets(buffer, BUFFER_LENGTH, fp);
		int length = strlen(buffer);
		INFO("length : %d\n", length);

        // name: qiuxiang,telephone: 98765678123
		char name[MAX_NAME] = {0};
		char phone[MAX_PHONE_NUMBER] = {0};

		if (0 != parser_token(buffer, length, name, phone)) {
			continue;
		}

        struct person *p = (struct person*)malloc(sizeof(struct person));
		if (p == NULL) return -2;

		memcpy(p->name, name, MAX_NAME);
		memcpy(p->phone, phone, MAX_PHONE_NUMBER);

		person_insert(ppeople, p);
		
		(*count) ++;
    }
    
}

int insert_entry(struct contacts* cts){
    //动态分配一个结构体内存p
    if(cts == NULL) return -1;
    struct person* p = (struct person*)malloc(sizeof(struct person));
    if(p == NULL) return -2;
    memset(p, 0, sizeof(struct person));

	//name
	INFO("Insert name:\n");
    scanf("%s", p->name);

	//phone,
	INFO("Insert phone:\n");
    scanf("%s", p->phone);

	//add people
    if(0 != person_insert(&cts->people, p)){
        free(p);
        return -3;
    }

    cts->count++;
    INFO("Insert SUCCESS!\n");
    return 0;

}

int print_entry(struct contacts* cts){
	if (cts == NULL) return -1;

	//cts->people
    person_traversal(cts->people);
}

int delete_entry(struct contacts* cts){
	//动态分配一个结构体内存p
    if(cts == NULL) return -1;

	//name
    char name[MAX_NAME];
	INFO("Delete name:\n");
    scanf("%s", name);

	//person_search
	struct person *p = person_search(cts->people, name);
    if (p == NULL){
        INFO("Person NOT Found!");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s", p->name, p->phone);
	//delete
    person_delete(&cts->people, p);
    free(p);

    cts->count--;
    INFO("Success Delete!");
    return 0;
	
}

int search_entry(struct contacts* cts){
	//动态分配一个结构体内存p
    if(cts == NULL) return -1;

    //input_name
	char name[MAX_NAME];
	INFO("Search name:\n");
    scanf("%s", name);

	//person_search
    struct person *p = person_search(cts->people, name);
    if (p == NULL){
        INFO("Person NOT Found!");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s", p->name, p->phone);

    return 0;
}

int save_entry(struct contacts *cts) {

	if (cts == NULL) return -1;

	INFO("Please Input Save Filename :\n");
	char filename[MAX_NAME] = {0};
	scanf("%s", filename);

	save_file(cts->people, filename);
	
}

int load_entry(struct contacts *cts) {
	if (cts == NULL) return -1;

	INFO("Please Input Load Filename :\n");
	char filename[MAX_NAME] = {0};
	scanf("%s", filename);

	load_file(&cts->people, &cts->count, filename);
}

void menu_info(void) {

	INFO("\n\n********************************************************\n");
	INFO("***** 1. Add Person\t\t2. Print People ********\n");
	INFO("***** 3. Del Person\t\t4. Search Person *******\n");
	INFO("***** 5. Save People\t\t6. Load People *********\n");
	INFO("***** Other Key for Exiting Program ********************\n");
	INFO("********************************************************\n\n");

}

int main(){
	//初始化
	struct contacts *cts = (struct contacts*)malloc(sizeof(struct contacts));
    if (cts == NULL) return -1;
    memset(cts, 0, sizeof(struct contacts));

    
	//业务循环
    while(1){
        menu_info();
        
        int insert = 0;
        scanf("%d", &insert);
        switch(insert){
            case OPER_INSERT:
				insert_entry(cts);
				break;

			case OPER_PRINT:
				print_entry(cts);
				break;

			case OPER_DELETE:
				delete_entry(cts);
				break;

			case OPER_SEARCH:
				search_entry(cts);
				break;

			case OPER_SAVE:
				save_entry(cts);
				break;

			case OPER_LOAD:
				load_entry(cts);
				break;

			default:
				goto exit;

        }
    }
    goto exit;
	//释放malloc分配的内存
exit:
    free(cts);
    return 0;
}
```



#### 总结:

1. 如何解决scanf输入内存溢出的问题?

   - 限制输入长度

   - 使用fgets替代scanf

     ```
     fgets(input, MAX_LENGTH, stdin);
     ```

   - 动态分配内存(malloc, realloc)

2. 二级指针的理解?

   - 函数内部修改传入的指针变量，

3. 做任何项目，一定要分层次去写，调试和使用代码会特别方便

#### 问题:

1. fopen为何打不开文件

   - 原因:文件权限不够,需要chmod添加读写权限,或者用sudo执行程序,这怎么办(问king老师)

   - 代码中加入错误判断,辅助帮助判断错误形式

   - ```
     if (fp == NULL){
             INFO("FILE %s can't open", filename);
             perror("Failed to open file");
             return -1;
         } 
     ```

     