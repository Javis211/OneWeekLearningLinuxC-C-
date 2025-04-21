## 【Linux入门环境编程】DNS协议与请求

### 1 DNS的介绍

​	域名系统（Domain Name System），人类可读域名和IP地址对应。

- 域名查询： `nslookup`
- 网络抓包： `wireshark`

#### DNS协议

### 头部

- **会话标识（2 字节）**：是 DNS 报文的 ID 标识，对于请求报文和其对应的应答报文，这个字段是相同的，通过它可以区分 DNS 应答报文是哪个请求的响应

- **标志（2 字节）**：

  | QR   | opcode | AA   | TC   | RD   | RA   | (zero) | rcode |
  | :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
  | 1    | 4    | 1    | 1    | 1    | 1    | 3    | 4    |
  
  - QR（1bit）：查询 / 响应标志，0 为查询，1 为响应
  - opcode（4bit）：0 表示标准查询，1 表示反向查询，2 表示服务器状态请求
  - AA（1bit）：表示授权回答
  - TC（1bit）：表示可截断的
  - RD（1bit）：表示期望递归
  - RA（1bit）：表示可用递归
  - rcode（4bit）：表示返回码，0 表示没有差错，3 表示名字差错，2 表示服务器错误（Server Failure）
  
- **数量字段（总共 8 字节）**：Questions、Answer RRs、Authority RRs、Additional RRs 各自表示后面的四个区域的数目。Questions 表示询问问题区域节的数量，Answers 表示回答区域的数量，Authoritative namesversers 表示授权区域的数量，Additional recoreds 表示附加区域的数量

![image-20250421111129988](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421111129988.png)

![image-20250421144544003](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421144544003.png)

- 域名结构：顶级域（TLD）,第二级域名（SLD）
- 域名服务器：顶级域名服务器，根域名服务器，运营商域名服务器，本地域名服务器

#### 域名解析过程：

- 静态域名：HOST文件
- 动态域名：需要查询

![image-20250421105956445](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421105956445.png)

流程如下：

1. 主机向本地域名服务器**递归查询**

2. 本地域名服务器使用**迭代查询**，告诉下一次应该查询根域名服务器
3. 根域名服务器告诉下一次应该查询顶级域名服务器
4. 顶级域名服务器告诉下一次应该查询阿里云（权限）域名服务器
5. （权限）域名服务器告诉IPv地址
6. 本地域名服务器吧结果告诉主机

### 2 代码实现DNS域名解析

#### DNS消息数据结构体

- 定义dns头部结构
- 域名查询query结构

#### 客户端向DNS服务器发送信息

##### 创建header

- 参数判断
- 根据时间种子生成一个随机值 `srandom`
- 把正常数据序列转换成网络序列 `htons`，设置flags和questions

##### 创建question

- 参数判断
- `name`的长度是 `hostname`的长度加2，为name分配内存

- 设置length，qtype， qclass的数据
- 定义分隔符为 `.` ，用于分割主机名。
- `strdup` 函数复制一份 `hostname` 字符串并返回新字符串的指针，这一步是为了在不破坏原始 `hostname` 字符串的情况下进行分割操作。（后文记得free掉内存）
- 使用 `strtok` 函数按 `.` 分割主机名，进入循环：
  - 每次获取一个分割后的子字符串（`token` ），获取其长度 `len` 。
  - 将子字符串长度存储到 `qname` 指向的位置，然后 `qname` 指针后移一位。
  - 使用 `strncpy` 将子字符串内容复制到 `qname` 指向的位置，`qname` 指针再后移子字符串长度的位数。（函数结尾已经把0赋值进入字符串了）
  - 继续调用 `strtok` 分割下一个子字符串，直到所有子字符串处理完。（注意strok不是线程安全的函数）

##### 把头部和question合并成一个请求

- 参数判断
- 把head数据拷贝到request里，记录偏置为offset
- 把question拷贝到request里，每次都记录偏置
- 返回偏置大小

##### 把DNS消息发送出去——使用UDP协议

- 使用 socket 函数创建一个 UDP 套接字，地址族为 AF_INET（IPv4 ）。如果创建失败（返回值小于 0 ），函数直接返回 -1 。
- 初始化一个 `sockaddr_in` 结构体来表示 DNS 服务器地址。设置地址族为 `AF_INET` ，端口号通过 `htons` 函数将主机字节序转换为网络字节序后赋值，IP 地址通过 `inet_addr` 函数将点分十进制字符串转换为二进制形式后赋值。
- 使用 `connect` 函数将创建的套接字连接到指定的 DNS 服务器。（在tcp编程中，connect为一次开路过程）
- 构建 DNS 请求头部和问题部分，并整合到一起
- 使用 recvfrom 函数从套接字接收响应数据，recvfrom 函数会填充发送方地址信息到 addr 结构体，
- 调用 dns_parse_response 函数（自定义函数）解析接收到的 DNS 响应报文，获取相关的解析结果（存储在 dns_domain 指针指向的结构体中）

##### DNS响应解析——不用理解，直接复制即可

1. `is_pointer`：用于判断 DNS 报文中的某个字段是否为指针。
2. `dns_parse_name`：用于解析 DNS 报文中的域名。
3. `dns_parse_response`：用于解析整个 DNS 响应报文，提取域名和对应的 IP 地址。

#### 完整代码

```
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #include <winsock2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#define DNS_SERVER_PORT		53
#define DNS_SERVER_IP		"114.114.114.114"

#define DNS_HOST			0x01
#define DNS_CNAME			0x05

// DNS头部
struct dns_header {
    unsigned short id;
	unsigned short flags;

	unsigned short questions; // 1
	unsigned short answer;

	unsigned short authority;
	unsigned short additional;

};

// DNS域名查询query
struct dns_question{
    int length;
    unsigned short qtype;
    unsigned short qclass;
	unsigned char *name; // 
};

struct dns_item {
	char *domain;
	char *ip;
};

// 客户端向DNS服务器发送
int dns_create_header(struct dns_header *header){
    if (header == NULL) return -1;
	memset(header, 0, sizeof(struct dns_header));

    // 生成一个随机值
	srandom(time(NULL));
	header->id = random();

    // 转换成网络序列
	header->flags = htons(0x0100);
	header->questions = htons(1); //注意要使用网络字节次序

    return 0;
}

// 创建查询序列
int dns_create_question(struct dns_question *question, const char *hostname) {

	if (question == NULL || hostname == NULL) return -1;
	memset(question, 0, sizeof(struct dns_question));

	question->name = (char*)malloc(strlen(hostname) + 2);
	if (question->name == NULL) {
		return -2;
	}
    
    // 数据设置
    question->length = strlen(hostname) + 2; 
	question->qtype = htons(1); 
	question->qclass = htons(1);

    // name设置
    // hostname: www.0voice.com
    // name: 3www60voice3com0
    const char delim[2] = "."; //定义分隔符为 `.` ，用于分割主机名。
	char *qname = question->name;
	
	char *hostname_dup = strdup(hostname); // strdup --> malloc，`strdup` 函数复制一份 `hostname` 字符串并返回新字符串的指针
	char *token = strtok(hostname_dup, delim); // www.0voice.com ,使用 strtok 函数按 . 分割主机名

    while (token != NULL) {
        // 每次获取一个分割后的子字符串（`token` ），获取其长度 `len` 。
		size_t len = strlen(token);
        // 将子字符串长度存储到 `qname` 指向的位置，然后 `qname` 指针后移一位
		*qname = len;
		qname ++;
        // 使用 `strncpy` 将子字符串内容复制到 `qname` 指向的位置，`qname` 指针再后移子字符串长度的位数。
		strncpy(qname, token, len+1);//函数结尾已经把0赋值进入字符串了
		qname += len;
        // 继续调用 `strtok` 分割下一个子字符串，直到所有子字符串处理完
		token = strtok(NULL, delim); //0voice.com ,  com

	}

    free(hostname_dup);
}

// struct dns_header *header
// struct dns_question *question
// char *request
// 把头部和question合并成一个请求
int dns_build_request(struct dns_header *header, struct dns_question *question, char *request, int rlen) {

	if (header == NULL || question == NULL || request == NULL) return -1;
	memset(request, 0, rlen);

	// header --> request，把head数据拷贝到request里，记录偏置为offset
	memcpy(request, header, sizeof(struct dns_header));
	int offset = sizeof(struct dns_header);

	// question --> request，把question拷贝到request里，每次都记录偏置
	memcpy(request+offset, question->name, question->length);
	offset += question->length;

	memcpy(request+offset, &question->qtype, sizeof(question->qtype));
	offset += sizeof(question->qtype);

	memcpy(request+offset, &question->qclass, sizeof(question->qclass));
	offset += sizeof(question->qclass);

	return offset;

}

//DNS响应解析——不用理解，直接复制即可
//判断DNS 报文中的某个字段是否为指针。
static int is_pointer(int in) {
	return ((in & 0xC0) == 0xC0);
}

// 解析 DNS 报文中的域名。
static void dns_parse_name(unsigned char *chunk, unsigned char *ptr, char *out, int *len) {

	int flag = 0, n = 0, alen = 0;
	char *pos = out + (*len);

	while (1) {

		flag = (int)ptr[0];
		if (flag == 0) break;

		if (is_pointer(flag)) {
			
			n = (int)ptr[1];
			ptr = chunk + n;
			dns_parse_name(chunk, ptr, out, len);
			break;
			
		} else {

			ptr ++;
			memcpy(pos, ptr, flag);
			pos += flag;
			ptr += flag;

			*len += flag;
			if ((int)ptr[0] != 0) {
				memcpy(pos, ".", 1);
				pos += 1;
				(*len) += 1;
			}
		}
	
	}
	
}
// 解析整个 DNS 响应报文，提取域名和对应的 IP 地址
static int dns_parse_response(char *buffer, struct dns_item **domains) {

	int i = 0;
	unsigned char *ptr = buffer;

	ptr += 4;
	int querys = ntohs(*(unsigned short*)ptr);

	ptr += 2;
	int answers = ntohs(*(unsigned short*)ptr);

	ptr += 6;
	for (i = 0;i < querys;i ++) {
		while (1) {
			int flag = (int)ptr[0];
			ptr += (flag + 1);

			if (flag == 0) break;
		}
		ptr += 4;
	}

	char cname[128], aname[128], ip[20], netip[4];
	int len, type, ttl, datalen;

	int cnt = 0;
	struct dns_item *list = (struct dns_item*)calloc(answers, sizeof(struct dns_item));
	if (list == NULL) {
		return -1;
	}

	for (i = 0;i < answers;i ++) {
		
		bzero(aname, sizeof(aname));
		len = 0;

		dns_parse_name(buffer, ptr, aname, &len);
		ptr += 2;

		type = htons(*(unsigned short*)ptr);
		ptr += 4;

		ttl = htons(*(unsigned short*)ptr);
		ptr += 4;

		datalen = ntohs(*(unsigned short*)ptr);
		ptr += 2;

		if (type == DNS_CNAME) {

			bzero(cname, sizeof(cname));
			len = 0;
			dns_parse_name(buffer, ptr, cname, &len);
			ptr += datalen;
			
		} else if (type == DNS_HOST) {

			bzero(ip, sizeof(ip));

			if (datalen == 4) {
				memcpy(netip, ptr, datalen);
				inet_ntop(AF_INET , netip , ip , sizeof(struct sockaddr));

				printf("%s has address %s\n" , aname, ip);
				printf("\tTime to live: %d minutes , %d seconds\n", ttl / 60, ttl % 60);

				list[cnt].domain = (char *)calloc(strlen(aname) + 1, 1);
				memcpy(list[cnt].domain, aname, strlen(aname));
				
				list[cnt].ip = (char *)calloc(strlen(ip) + 1, 1);
				memcpy(list[cnt].ip, ip, strlen(ip));
				
				cnt ++;
			}
			
			ptr += datalen;
		}
	}

	*domains = list;
	ptr += 2;

	return cnt;
	
}

// 把DNS消息发送出去——使用UDP协议
int dns_client_commit(const char *domain) {
    // 使用 socket 函数创建一个 UDP 套接字，地址族为 AF_INET（IPv4)
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		return  -1;
	}

    //初始化一个 sockaddr_in 结构体来表示 DNS 服务器地址
	struct sockaddr_in servaddr = {0};
    //设置地址族为 AF_INET 
	servaddr.sin_family = AF_INET;
    //端口号通过 htons 函数将主机字节序转换为网络字节序后赋值
	servaddr.sin_port = htons(DNS_SERVER_PORT);
    //IP 地址通过 inet_addr 函数将点分十进制字符串转换为二进制形式后赋值
	servaddr.sin_addr.s_addr = inet_addr(DNS_SERVER_IP);

    // 使用 connect 函数将创建的套接字连接到指定的 DNS 服务器。
	int ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	printf("connect : %d\n", ret);

	// 构建请求
    struct dns_header header = {0};
	dns_create_header(&header);

	struct dns_question question = {0};
	dns_create_question(&question, domain);

    // 定义一个缓冲区 request 用于存储构建好的 DNS 请求报文，
    // 调用 dns_build_request 函数（自定义函数）将头部和问题部分组合成完整的请求报文
	char request[1024] = {0};
	int length = dns_build_request(&header, &question, request, 1024);

	// request
    // 使用 sendto 函数通过 UDP 套接字向 DNS 服务器发送构建好的请求报文，sendto 函数返回实际发送的字节数
	int slen = sendto(sockfd, request, length, 0, (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
	
	//recvfrom 
    // 定义缓冲区 response 用于存储接收的 DNS 响应报文
	char response[1024] = {0};
	struct sockaddr_in addr;
	size_t addr_len = sizeof(struct sockaddr_in);
	
	// 使用 recvfrom 函数从套接字接收响应数据，recvfrom 函数会填充发送方地址信息到 addr 结构体，
	int n = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);

    // printf("recvfrom: %d, %s\n", n, response);
    //DNS 解析相应报文
	struct dns_item *dns_domain = NULL;
	dns_parse_response(response, &dns_domain);
	free(dns_domain);
	
	return n;

}

int main(int argc, char *argv[]) {

	if (argc < 2) return -1;

	dns_client_commit(argv[1]);

}
```



##### 调试问题

，需要加入以下头文件进行显式声明，防止warning：

```
#include <time.h>
#include <arpa/inet.h>
```

### 3 总结

1. UDP编程的好处：
   - 传输速度快（下载）
   - 响应速度快（游戏）

## 【Linux入门环境编程】Http客户端请求

### 1 Http协议的介绍

HTTP 是一个基于 TCP/IP 通信协议，在TCP连接，socket连接的基础上来传递数据的协议（首先要建立tcp连接）

- HTTP 是无连接：无连接的含义是限制每次连接只处理一个请求。服务器处理完客户的请求，并收到客户的应答后，即断开连接。采用这种方式可以节省传输时间。

#### 客户端请求信息

![image-20250421163723184](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421163723184.png)

```
GET /hello.txt HTTP/1.1
User-Agent: curl/7.16.3 libcurl/7.16.3 0penssL/0.9.7l zlib/1.2.3
Host: www.example.com
Accept-Language: en, mi
```

HTTP1.0 定义了三种请求方法： 

- GET, 
- POST 和 
- HEAD 方法。

HTTP1.1 新增了六种请求方法：

- OPTIONS、
- PUT、
- PATCH、
- DELETE、
- TRACE 和 
- CONNECT 方法。

#### 服务器响应信息

![image-20250421164244259](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421164244259.png)

```
HTTP/1.1 200 OK 
Date: Mon, 27 Jul 2009 12:28:53 GMT 
Server: Apache
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT 
ETag: "34aa387-d-1568eb00"
Accept-Ranges: bytes 
Content-Length: 51
Vary: Accept-Encoding 
Content-Type: text/plain
```

### 2 代码实现

#### 实现dns解析：

- `struct hostent *host_entry = gethostbyname`DNS功能，把域名转换成 `unsigned int`的IP地址
- `inet_ntoa`把`unsigned int`的IP地址转化成人可读的字符串
- `inet_ntoa` 函数用于将网络字节序的 IPv4 地址转换为点分十进制的字符串表示

#### 创建TCP连接：

- 创建套接字 `socket(AF_INET, SOCK_STREAM, 0)`
- 初始化服务器地址结构体 :
  - IPV4 -- `sin_family = AF_INET`
  - 端口80 -- `sin_port`
  - 把点分十进制转换成网络字节序的 IPv4 地址 `inet_addr`

- 连接到服务器

- 设置套接字为非阻塞模式 `fcntl(sockfd, F_SETFL, O_NONBLOCK)`
  - O_NONBLOCK 表示将套接字设置为非阻塞模式，即在进行网络操作时不会阻塞程序的执行；而阻塞模式标识如果read（）没有读取到返回，线程会被挂起，直到等到read中有数据返回
  - `fcntl` 是一个系统调用，用于对文件描述符进行各种控制操作。
  - `F_SETFL` 表示设置文件描述符的状态标志。

- 返回sockfd，他是一个文件描述符，可以直接返回

#### 发送HTTP请求：

- hostname：如 "github.com"； resource：如 "/hsy" 表示根目录
- 解析主机名到 IP 地址，函数创建一个 TCP 套接字并连接到指定 IP 地址的 HTTP 服务器
- 使用格式化字符串写入字符buffer：
  - \在句尾作为占意字符
  - `CONNETION_TYPE    "Connection: close\r\n"`操作连接即刻中断

- 发送 HTTP 请求`send()`
- select 进行 I/O 多路复用,监听I/O里面有没有数据:
  - `fd_set`：定义一个文件描述符集合 `fdread`，用于存储需要监视的文件描述符。
  - `FD_ZERO(&fdread)`：将文件描述符集合 `fdread` 初始化为空。
  - `FD_SET(sockfd, &fdread)`：将套接字描述符 `sockfd` 添加到文件描述符集合 `fdread` 中，表示要监视该套接字的可读事件。
  - `struct timeval tv`：定义一个时间结构体 `tv`，用于设置 `select` 函数的超时时间。这里将超时时间设置为 5 秒。
- 循环接收服务器响应:
  - `select(sockfd+1, &fdread, NULL, NULL, &tv)`：调用 `select` 函数监视文件描述符集合 `fdread` 中的套接字 `sockfd` 是否有可读事件发生。`sockfd+1` 表示要监视的最大文件描述符加 1。如果超时或没有可读事件发生，`select` 函数返回 0；如果有错误发生，返回 -1；如果有可读事件发生，返回就绪的文件描述符数量。
  - `FD_ISSET(sockfd, &fdread)`：检查套接字描述符 `sockfd` 是否在文件描述符集合 `fdread` 中且有可读事件发生。
  - `recv(sockfd, buffer, BUFFER_SIZE, 0)`：使用 `recv` 函数从套接字 `sockfd` 接收服务器的响应数据，并将其存储在 `buffer` 中。`len` 表示实际接收到的字节数。
  - `realloc(result, (strlen(result) + len + 1) * sizeof(char))`：使用 `realloc` 函数重新分配内存空间，以容纳新接收到的响应数据。
  - `strncat(result, buffer, len)`：将新接收到的响应数据追加到 `result` 中。

#### 完整代码：

```
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #include <winsock2.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define HTTP_VERSION		"HTTP/1.1"
#define CONNETION_TYPE		"Connection: close\r\n"

#define BUFFER_SIZE		     4096

// DNS --> 
// baidu --> struct hosten
// 上一节中的DNS实习
char *host_to_ip(const char *hostname) {
    //DNS功能，把域名转换成 `unsigned int`的IP地址
	struct hostent *host_entry = gethostbyname(hostname); //dns

	// 14.215.177.39 --> 
	//inet_ntoa ( unsigned int --> char *
	// 0x12121212 --> "18.18.18.18"
	if (host_entry) {
		// inet_ntoa 函数用于将网络字节序的 IPv4 地址转换为点分十进制的字符串表示
        return inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);
        // h_addr_list：是一个指针数组，每个元素指向一个 IP 地址，一个主机名可能对应多个 IP 地址
    } 

	return NULL;
}

int http_create_socket(char *ip) {
    // 创建套接字
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化服务器地址结构体
	struct sockaddr_in sin = {0};
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80); //
 	sin.sin_addr.s_addr = inet_addr(ip);
    //  连接到服务器
	if (0 != connect(sockfd, (struct sockaddr*)&sin, sizeof(struct sockaddr_in))) {
		return -1;
	}
    // 设置套接字为非阻塞模式
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
    // fcntl 是一个系统调用，用于对文件描述符进行各种控制操作。
    // F_SETFL 表示设置文件描述符的状态标志。
    // O_NONBLOCK 表示将套接字设置为非阻塞模式，即在进行网络操作时不会阻塞程序的执行。
	
    // 返回的是一个套接字描述符
    return sockfd;

}

// hostname : github.com --> 
// resource ：/hsy
char * http_send_request(const char *hostname, const char *resource) {
    // 解析主机名到 IP 地址
	char *ip = host_to_ip(hostname); 
    // 函数创建一个 TCP 套接字并连接到指定 IP 地址的 HTTP 服务器 
	int sockfd = http_create_socket(ip);

    // 构造 HTTP 请求消息
	char buffer[BUFFER_SIZE] = {0};
	sprintf(buffer, 
        //'\'在句尾作为占意字符
        // `CONNETION_TYPE    "Connection: close\r\n"`操作连接即刻中断
"GET %s %s\r\n\
Host: %s\r\n\
%s\r\n\
\r\n",

	resource, HTTP_VERSION,
	hostname,
	CONNETION_TYPE
	);
    // 发送 HTTP 请求
	send(sockfd, buffer, strlen(buffer), 0);

	//select 函数进行 I/O 多路复用,监听I/O里面有没有数据
    // 定义一个文件描述符集合 `fdread`，用于存储需要监视的文件描述符
	fd_set fdread;
	
	FD_ZERO(&fdread);//将文件描述符集合 `fdread` 初始化为空。
	FD_SET(sockfd, &fdread);//将套接字描述符 `sockfd` 添加到文件描述符集合 `fdread` 中。
    
    // 将套接字描述符 `sockfd` 添加到文件描述符集合 `fdread` 中，表示要监视该套接字的可读事件。
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

    // 分配内存用于存储响应结果
	char *result = malloc(sizeof(int));
	memset(result, 0, sizeof(int));

	// 循环接收服务器响应
	while (1) {
        // select(masxfd+1, &rset, &wset, &eset, NULL);
        // 第一个参数要监视的文件描述符，第二个参数哪个IO可读，第3个参数哪个IO可读，第4个参数哪个IO出错，第5个参数限制时间
		int selection = select(sockfd+1, &fdread, NULL, NULL, &tv);
		// 如果超时或没有可读事件发生，select 函数返回 0；
        // 如果有错误发生，返回 -1；
        // 如果有可读事件发生，返回就绪的文件描述符数量。
        // FD_ISSET(sockfd, &fdread)：检查套接字描述符 sockfd 是否在文件描述符集合 fdread 中且有可读事件发生。
        if (!selection || !FD_ISSET(sockfd, &fdread)) {
			break;
		} else {
            // 清空buffer
			memset(buffer, 0, BUFFER_SIZE);
            // 使用 recv 函数从套接字 sockfd 接收服务器的响应数据，并将其存储在 buffer 中。
            // len 表示实际接收到的字节数。
			int len = recv(sockfd, buffer, BUFFER_SIZE, 0);
			if (len == 0) { // disconnect
				break;
			}
            // 使用 realloc 函数重新分配内存空间，以容纳新接收到的响应数据。
			result = realloc(result, (strlen(result) + len + 1) * sizeof(char));
            // 将新接收到的响应数据追加到 result 中
			strncat(result, buffer, len);
		}

	}

	return result;
}

int main(int argc, char *argv[]) {

	if (argc < 3) return -1;

	char *response = http_send_request(argv[1], argv[2]);
	printf("response : %s\n", response);

	free(response);
	
}
```

## 【Linux入门环境编程】TCP服务器实现

### 0 目录

1. 基础部分——网络编程
2. 并发服务器
   1. 一请求一线程
   2. IO多路复用，epoll
3. TCP服务器百万级连接 

### 1 TCP服务器介绍



### 2 1请求1连接代码实现

#### 监听客户端线程：

1. 总体采用一请求一线程方式，可以设置阻塞
2. 使用recv收到数据，进行判断和打印

#### 实现TCP服务器的监听功能：

1. 设置socket连接，绑定端口、ip
2. 监听端口，进入循环
3. `accept` 存储客户端的地址信息以及地址结构体的长度
4. 开启一个新的线程

#### 完整代码

```
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <unistd.h>

#define BUFFER_LENGTH             1024

void *client_routine(void *arg) {

	int clientfd = *(int *)arg;

	while (1) {

		char buffer[BUFFER_LENGTH] = {0};
		int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
		if (len < 0) { //表示 recv 函数调用出错
			close(clientfd);
			break;
		} else if (len == 0) { // disconnect表示客户端已经正常关闭了连接
			close(clientfd);
			break;
		} else {
			printf("Recv: %s, %d byte(s)\n", buffer, len);
		}

	}

}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Param Error\n");
		return -1;
	}
    // 字符串转换为整数（int类型）“ASCII to integer”
	int port = atoi(argv[1]);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY; //0.0.0.0 任何一台电脑都可以连接

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return 2;
	}
    // listen 函数将套接字 sockfd 从主动模式转换为被动模式，使其可以接受客户端的连接请求。
    // 5 是监听队列的最大长度
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		return 3;
	}
	// 
    while (1) {
        // 用于存储客户端的地址信息，包括客户端的 IP 地址和端口号
		struct sockaddr_in client_addr;
		memset(&client_addr, 0, sizeof(struct sockaddr_in));
		// socklen_t 是一个无符号整数类型，用于表示地址结构体的长度
        socklen_t client_len = sizeof(client_addr);

		int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
		
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, client_routine, &clientfd);

	}

    return 0;
}
```



#### 测试（1请求1线程）

使用网络调试助手netassitant调试界面

![image-20250421222015566](C:\Users\DHFK.601\AppData\Roaming\Typora\typora-user-images\image-20250421222015566.png)

调试成功

问题：

- 如果有100w个客户端连接服务器，还要开100w个线程吗？

### 3 Epoll多路复用介绍

- **正经描述**：实现对多个文件描述符的监控，以确定哪些文件描述符上有事件发生，从而实现高效的 I/O 操作

- **抽象描述**：就像是在小区门口（服务器）的快递员，谁家有快递就去谁家送
- **内核事件表**：epoll 会在内核中创建一个事件表，用于存储需要监控的文件描述符及其相关事件。通过 `epoll_create` 函数创建一个 epoll 实例，得到一个 epoll 文件描述符，该描述符就代表了这个内核事件表。
- **添加、修改和删除文件描述符**：使用 `epoll_ctl` 函数可以向 epoll 实例中添加、修改或删除要监控的文件描述符及其感兴趣的事件（如可读、可写、异常等）。当有文件描述符就绪时，内核会将其放入一个就绪队列中。
- **等待事件发生**：应用程序通过调用 `epoll_wait` 函数阻塞等待事件发生。`epoll_wait` 函数会返回就绪的文件描述符集合，应用程序可以遍历这个集合，对就绪的文件描述符进行相应的 I/O 操作。
- **监听套接字的作用**：监听套接字 sockfd 是服务器端用于监听特定端口，等待客户端连接请求的套接字。它被创建并绑定到指定的端口后，就会一直处于监听状态，等待客户端的连接请求。
- **事件驱动机制的工作原理**：epoll 是一种事件通知机制，它会监视一组文件描述符（包括套接字）的事件发生情况。当有事件发生时，epoll 会将相关的事件信息填充到 events 数组中，并返回给应用程序。这里的 events[i].data.fd 就是发生事件的文件描述符，当它与监听套接字 sockfd 相等时，意味着发生事件的是监听套接字。
- **新连接请求的处理流程**：当客户端发起连接请求时，服务器端的监听套接字会接收到这个连接请求事件。epoll 检测到监听套接字上的连接请求事件后，就会通过 events 数组通知应用程序。应用程序通过检查 events[i].data.fd 是否等于 sockfd，来判断是否是新的客户端连接请求。如果相等，就可以调用 accept 函数来接受这个连接请求，获取与客户端通信的新套接字。

### 4 Epoll代码实现

#### 初始化epoll：

- 创建 epoll 实例
- 定义 epoll 事件数组
- 准备监听套接字的事件并添加到 epoll 实例：
  - 有新的客户端连接请求到来
  - 设置为监听套接字的文件描述符 sockfd
  - 将监听套接字 sockfd 添加到 epoll 实例中

- 进入事件循环，等待事件发生，超时时间设置为 5 秒，如果什么事件都没发生，继续循环

- 如果events[i].data.fd 等于监听套接字 sockfd，说明有新的客户端连接请求到来：
  - 使用 `accept` 函数接受新的连接，得到客户端套接字描述符 `clientfd`
  - 设置要监听的事件为 `EPOLLIN`（可读事件）（有数据）和 `EPOLLET`（边缘触发模式）（数据从无到有）
  - 将客户端套接字 `clientfd` 添加到 `epoll` 实例中，监听其可读事件。
- 如果 events[i].data.fd 不等于监听套接字 sockfd，说明是某个客户端套接字有可读事件：
  - 从客户端套接字 `clientfd` 接收数据
  - 接收数据时发生错误，关闭客户端套接字，并使用 `epoll_ctl` 函数将其从 `epoll` 实例中删除。
  - 客户端关闭了连接，同样关闭客户端套接字，并将其从 `epoll` 实例中删除。

#### 完整代码：

```
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <unistd.h>

#define BUFFER_LENGTH             1024
#define EPOLL_SIZE			      1024

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("Param Error\n");
		return -1;
	}
    // 字符串转换为整数（int类型）“ASCII to integer”
	int port = atoi(argv[1]);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY; //0.0.0.0 任何一台电脑都可以连接

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		return 2;
	}
    // listen 函数将套接字 sockfd 从主动模式转换为被动模式，使其可以接受客户端的连接请求。
    // 5 是监听队列的最大长度
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		return 3;
	}
    // 创建 epoll 实例
    int epfd = epoll_create(1);  //epoll_create 的参数会被忽略，但必须为一个大于 0 的值，这里传入 1
	// 定义 epoll 事件数组
    struct epoll_event events[EPOLL_SIZE] = {0};
    // 准备监听套接字的事件并添加到 epoll 实例
	struct epoll_event ev;
	ev.events = EPOLLIN; //有新的客户端连接请求到来
	ev.data.fd = sockfd; //设置为监听套接字的文件描述符 sockfd
    // 将监听套接字 sockfd 添加到 epoll 实例中，EPOLL_CTL_ADD 表示添加操作
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	
	while (1) {
        // 等待事件发生，超时时间设置为 5 秒
		int nready = epoll_wait(epfd, events, EPOLL_SIZE, 5); 
        // -1一直不去, 0只要有时间就去, 5毫秒没有事件就返回0
		if (nready == -1) continue; //什么事件都没发生，继续循环
        // 返回激活的快递员数量

		int i = 0;
		for (i = 0;i < nready;i ++) {
            // 如果 events[i].data.fd 等于监听套接字 sockfd，说明有新的客户端连接请求到来
			if (events[i].data.fd == sockfd) { // listen 

				struct sockaddr_in client_addr;
				memset(&client_addr, 0, sizeof(struct sockaddr_in));
				socklen_t client_len = sizeof(client_addr);
                // 使用 accept 函数接受新的连接，得到客户端套接字描述符 clientfd
				int clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
                // 设置要监听的事件为 EPOLLIN（可读事件）和 EPOLLET（边缘触发模式）
				ev.events = EPOLLIN | EPOLLET; 
				ev.data.fd = clientfd;
				// 将客户端套接字 clientfd 添加到 epoll 实例中，监听其可读事件。
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

			} else { //如果 events[i].data.fd 不等于监听套接字 sockfd，说明是某个客户端套接字有可读事件。

				int clientfd = events[i].data.fd;
				
				char buffer[BUFFER_LENGTH] = {0};
                // 从客户端套接字 clientfd 接收数据
				int len = recv(clientfd, buffer, BUFFER_LENGTH, 0);
				if (len < 0) { //关闭客户端套接字，并使用 epoll_ctl 函数将其从 epoll 实例中删除。
					close(clientfd);

					ev.events = EPOLLIN; 
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
					
				} else if (len == 0) { // disconnect客户端关闭了连接，同样关闭客户端套接字，并将其从 epoll 实例中删除。
					close(clientfd);

					ev.events = EPOLLIN; 
					ev.data.fd = clientfd;
					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);
					
				} else {
					printf("Recv: %s, %d byte(s)\n", buffer, len);
				}
				
				
			}

		}

	}





#endif
    return 0;
}
```

#### 总结：

- 面试时说明白epoll的水平与边沿触发
- 开发时注意判断 `sockfd`在不在`epoll`事件集合中
- 后期课程会讲到`reactor`包装 `epoll`，以及协程事件（epoll的更优秀的封装）
- 服务器是避免不了 `epoll`的事件主循环
- 下一次实现百万级别的连接