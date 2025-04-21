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