#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUF_SIZE 1024

static pthread_t thr;
static int thr_exit = 1;
static int thr_id;
static char recv_data[BUF_SIZE];
static struct sockaddr_in server_addr, client_addr;
static int server_fd, client_fd, n, n2;
static void *treturn;

void *thread_recv(void *arg);

void thread_start() 
{
	thr_exit = 0;
	thr_id = pthread_create(&thr, NULL, thread_recv, NULL);
}

void thread_stop()
{
	thr_exit = 1;
	thr_id = pthread_join(thr, &treturn);
}

void *thread_recv(void *arg)
{
	while(!thr_exit) {
		if((n = recv(client_fd, recv_data, sizeof(recv_data),0)) == -1) {
			printf("[client: %s(%d)] disconnected!!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			thread_stop();		
			close(client_fd);
		}
		else if(n > 0) {
			recv_data[n] = '\0';
			printf("[client: %s(%d)]: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_data);
		}
	}

	pthread_exit((void *)0);
}

int main(int argc, char *argv[]) 
{
	char chat_data[BUF_SIZE];
	char temp[20];
	int len;

	if(argc != 2) {
		printf("Usage: %s [PORT]\n", argv[0]);
		exit(-1);
	}

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd < 0) {
		printf("Server: can not open Socket!\n");
		exit(-1);
	}
	memset(&server_addr, 0x00, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: can not bind local address!\n");
		exit(0);
	}

	if(listen(server_fd, 5) < 0) {
		printf("Server: can not listen connect!\n");
		exit(0);
	}

	memset(recv_data, 0x00, sizeof(recv_data));
	len = sizeof(client_addr);
	printf("==== [PORT] : %d ====\n", atoi(argv[1]));
	printf("Server : waiting connection request.\n");


	while(1) {
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);
		if(client_fd < 0) {
			printf("Server: accept failtd\n");
			exit(0);
		}
		inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
		printf("Server: %s clinect connected.\n", temp);

		printf("\n%s(%d) entered. Enter 'quit' to exit!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		while(1) {
			thread_start();	
			fgets(chat_data, sizeof(chat_data), stdin);
			n2 = send(client_fd, chat_data, sizeof(chat_data),0);
			if(n2 < 0) {
				printf("Server: send error!! n2 = %d\n", n2);
				break;
			}
		}
		thread_stop();
		close(client_fd);
		printf("Server: %s client closed.\n", temp);
	}
	close(server_fd);
	return 0;
}



