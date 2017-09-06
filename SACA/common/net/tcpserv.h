#ifndef TCP_SERV_H
#define TCP_SERV_H

#include "tcpcon.h"

typedef struct tcpserv tcpserv;
typedef void (*con_handler_t)(tcpcon*);

struct tcpserv
{
    int sockfd;
    struct sockaddr_in addr;
    pthread_t listenerthread;
    con_handler_t con_handler;
    msg_handler_t msg_handler;
    close_handler_t close_handler;
    unsigned int isrunning;
};

tcpserv* tcpserv_create(const char* hostname, int port, con_handler_t con_handler, msg_handler_t msg_handler, close_handler_t close_handler);
void tcpserv_free(tcpserv* serv);
void tcpserv_start(tcpserv* serv);
void tcpserv_stop(tcpserv* serv);

#endif
