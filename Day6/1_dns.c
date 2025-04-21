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