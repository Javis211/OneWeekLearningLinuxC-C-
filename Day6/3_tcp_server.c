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

#if 0
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
#else
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