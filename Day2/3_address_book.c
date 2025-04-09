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
    if (fp == NULL){
        INFO("FILE %s can't open", filename);
        perror("Failed to open file");
        return -1;
    } 
	
    //循环文件写入
    struct person* temp = NULL;
    for (temp = people; temp != NULL; temp = temp->next){
        fprintf(fp, "name: %s, phone: %s", temp->name, temp->phone);
        fflush(fp);
    }

    fclose(fp);
    return 0;
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

    fclose(fp);
    return 0;
    
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
        INFO("Person NOT Found!\n");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s\n", p->name, p->phone);
	//delete
    person_delete(&cts->people, p);
    free(p);

    cts->count--;
    INFO("Success Deleten\n");
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
        INFO("Person NOT Found!\n");
        free(p);
        return -2;
    }
    INFO("Found name: %s, phone: %s\n", p->name, p->phone);

    return 0;
}

int save_entry(struct contacts *cts) {

	if (cts == NULL) return -1;

	INFO("Please Input Save Filename :\n");
	char filename[MAX_NAME] = {0};
	scanf("%s", filename);

	if(!save_file(cts->people, filename)){
        INFO("Success SaveFile!");
    }
	
}

int load_entry(struct contacts *cts) {
	if (cts == NULL) return -1;

	INFO("Please Input Load Filename :\n");
	char filename[MAX_NAME] = {0};
	scanf("%s", filename);

	if(!load_file(&cts->people, &cts->count, filename)){
        INFO("Success LoadFile!");
    }
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