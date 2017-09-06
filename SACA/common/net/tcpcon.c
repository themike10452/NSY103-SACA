#include "tcpcon.h"
#include "../utils/stringutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

void* readthread_main(void* args);
void* writethread_main(void* args);
void* pingthread_main(void* args);

tcpcon* tcpcon_open(const char* host, int port, msg_handler_t msg_handler, close_handler_t close_handler)
{
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);;

    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
        perror("Connection failed\n");
		return NULL;
	}

	return tcpcon_create(sockfd, &addr, msg_handler, close_handler);
}

tcpcon* tcpcon_create(int socket, struct sockaddr_in* addr, msg_handler_t msg_handler, close_handler_t close_handler)
{
    tcpcon* con = (tcpcon*)calloc(1, sizeof(tcpcon));
	con->sockfd = socket;

	// socket receive timeout
	struct timeval tv;
	tv.tv_sec = TCPCON_SOCKET_TIMEOUT;
	tv.tv_usec = 0;
    setsockopt(con->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // no delay
    int one = 1;
    setsockopt(con->sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(int));

	con->server.sin_addr.s_addr = addr->sin_addr.s_addr;
	con->server.sin_port = addr->sin_port;
	con->server.sin_family = addr->sin_family;

	pthread_mutex_init(&con->mutex, NULL);
	pthread_cond_init(&con->cond_isclosing, NULL);
	pthread_cond_init(&con->cond_msginqueue, NULL);

	con->write_queue = list_create(10);
	con->msg_handler = msg_handler;
	con->close_handler = close_handler;

	con->isalive = 1;

	pthread_create(&con->readthread, NULL, readthread_main, con);
	pthread_create(&con->writethread, NULL, writethread_main, con);
	pthread_create(&con->pingthread, NULL, pingthread_main, con);

	return con;
}

void tcpcon_close(tcpcon* c)
{
	if (!c->isalive)
		return;

	pthread_mutex_lock(&c->mutex);
		c->isalive = 0;
		shutdown(c->sockfd, SHUT_RDWR);

		if (pthread_self() != c->readthread)
			pthread_join(c->readthread, NULL);
		
		list_clear(c->write_queue, YES);

		pthread_cond_signal(&c->cond_msginqueue);
		pthread_cond_signal(&c->cond_isclosing);
	pthread_mutex_unlock(&c->mutex);
	
	pthread_join(c->writethread, NULL);
	pthread_join(c->pingthread, NULL);

	if (c->close_handler)
		c->close_handler(c);
}

void tcpcon_free(tcpcon* c)
{
	free(c);
}

void tcpcon_sendstr(const tcpcon* c, const char* msg, long len)
{
	if (!c->isalive) return;

	pthread_mutex_lock(&c->mutex);
		if (len == -1)
			len = strlen(msg);
		char* msgcpy = (char*)malloc(len + 1ul);
		str_cpy(msgcpy, msg, len);
		list_push(c->write_queue, msgcpy);
		pthread_cond_signal(&c->cond_msginqueue);
	pthread_mutex_unlock(&c->mutex);
}

void tcpcon_sendmsg(const tcpcon* c, const msg_t* msg)
{
	char* str = msg_serialize(msg);
	tcpcon_sendstr(c, str, strlen(str));
	free(str);
}

void* readthread_main(void* args)
{
	tcpcon* con = (tcpcon*)args;

	int read;
	char buffer[TCPCON_READ_BUFFER_SIZE];

	while ((read = recv(con->sockfd, buffer, TCPCON_READ_BUFFER_SIZE, 0)) > 0)
	{
		if (con->msg_handler && strcmp(buffer, "\n") != 0)
			con->msg_handler(con, buffer);

        memset(buffer, 0, sizeof(buffer));
	}

	tcpcon_close(con);
}

void* writethread_main(void* args)
{
	tcpcon* con = (tcpcon*)args;

	pthread_mutex_lock(&con->mutex);
		char* msg;
		int count;
		while (con->isalive)
		{
			// wait for queue input
			if (con->write_queue->size == 0)
				pthread_cond_wait(&con->cond_msginqueue, &con->mutex);

			if (con->isalive && (count = con->write_queue->size) > 0)
			{
				for (int i = 0; i < count; i++)
				{
					msg = (char*)list_get(con->write_queue, i);
					send(con->sockfd, msg, strlen(msg), 0);
				}

				list_clear(con->write_queue, YES);
			}
		}
	pthread_mutex_unlock(&con->mutex);
}

void* pingthread_main(void* args)
{
	tcpcon* con = (tcpcon*)args;

	struct timespec t;
	t.tv_sec = time(NULL) + TCPCON_PING_INTERVAL;
	t.tv_nsec = 0;

	int waitres;
	while (con->isalive)
	{
		pthread_mutex_lock(&con->mutex);
			waitres = pthread_cond_timedwait(&con->cond_isclosing, &con->mutex, &t);
		pthread_mutex_unlock(&con->mutex);
		
		if (waitres == ETIMEDOUT)
		{
			tcpcon_sendstr(con, "\n", -1);
			t.tv_sec = time(NULL) + TCPCON_PING_INTERVAL;
		}
	}
}
