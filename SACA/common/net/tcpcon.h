#ifndef TCPCON_H
#define TCPCON_H

#include "../utils/list.h"
#include "message.h"

#include <pthread.h>
#include <netinet/in.h>

#define TCPCON_READ_BUFFER_SIZE 2000
#define TCPCON_SOCKET_TIMEOUT 15
#define TCPCON_PING_INTERVAL 5

typedef struct tcpcon tcpcon;
typedef void (*msg_handler_t)(tcpcon*, char*);
typedef void (*close_handler_t)(tcpcon*);

struct tcpcon
{
	int sockfd;
	struct sockaddr_in server;
	pthread_t readthread;
	pthread_t writethread;
	pthread_t pingthread;
	pthread_mutex_t mutex;
	pthread_cond_t cond_isclosing;
	pthread_cond_t cond_msginqueue;
	list_t* write_queue;
	msg_handler_t msg_handler;
	close_handler_t close_handler;
	unsigned int isalive;
};

tcpcon* tcpcon_open(const char* host, int port, msg_handler_t msg_handler, close_handler_t close_handler);
tcpcon* tcpcon_create(int socket, struct sockaddr_in* addr, msg_handler_t msg_handler, close_handler_t close_handler);
void tcpcon_close(tcpcon* c);
void tcpcon_free(tcpcon* c);
void tcpcon_sendstr(const tcpcon* c, const char* msg, long len);
void tcpcon_sendmsg(const tcpcon* c, const msg_t* msg);

#endif
