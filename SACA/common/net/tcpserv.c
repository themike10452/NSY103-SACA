#include "tcpserv.h"

#include <stdlib.h>
#include <arpa/inet.h>

void* listenerthread_main(void* args);

tcpserv* tcpserv_create(const char* hostname, int port, con_handler_t con_handler, msg_handler_t msg_handler, close_handler_t close_handler)
{
    tcpserv* serv = (tcpserv*)calloc(1, sizeof(tcpserv));
    serv->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    int one = 1;
    setsockopt(serv->sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));
    setsockopt(serv->sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    serv->addr.sin_family = AF_INET;
    serv->addr.sin_addr.s_addr = inet_addr(hostname);
    serv->addr.sin_port = htons(port);

    if (bind(serv->sockfd, (struct sockaddr*)&serv->addr, sizeof(serv->addr)) < 0)
    {
        return NULL;
    }

    serv->con_handler = con_handler;
    serv->msg_handler = msg_handler;
    serv->close_handler = close_handler;

    return serv;
}

void tcpserv_free(tcpserv* serv)
{
    free(serv);
}

void tcpserv_start(tcpserv* serv)
{
    if (serv->isrunning)
        return;
    
    serv->isrunning = 1;

    pthread_create(&serv->listenerthread, NULL, listenerthread_main, serv);
}

void tcpserv_stop(tcpserv* serv)
{
    if (!serv->isrunning)
        return;

    serv->isrunning = 0;
    shutdown(serv->sockfd, SHUT_RDWR);

    pthread_join(serv->listenerthread, NULL);
}

void* listenerthread_main(void* args)
{
    tcpserv* serv = (tcpserv*)args;

    int socket;
    struct sockaddr_in client;
    socklen_t len = (socklen_t)sizeof(client);   

    listen(serv->sockfd, 5);

    while (serv->isrunning)
    {
        socket = accept(serv->sockfd, (struct sockaddr*)&client, &len);
        if (socket >-1 && serv->con_handler)
        {
            tcpcon* con = tcpcon_create(socket, &client, serv->msg_handler, serv->close_handler);
            serv->con_handler(con);
        }
    }
}
