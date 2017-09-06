#include "controller.h"
#include "airplane.h"
#include "../common/net/tcpcon.h"
#include "../common/net/tcpserv.h"
#include "../common/utils/pair.h"
#include "../common/utils/base64.h"
#include "collisiondetection.h"

#include <unistd.h>
#include <signal.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

list_t* airplanes;
list_t* cmd_consoles;
list_t* mon_consoles;

tcpserv* serv_airplanes;
tcpserv* serv_cmd;
tcpserv* serv_mon;

int keepalive = 1;

int init();
void broadcast();
void detect_collisions();
char* serialize_airplane_list();
airplane* find_airplane(int socket);
void add_airplane(int socket, airplane* ap);
void remove_airplane(int socket);
void ap_handle_connect(tcpcon* con);
void ap_handle_message(tcpcon* con, char* msg);
void ap_handle_disconnect(tcpcon* con);
void mon_handle_connect(tcpcon* con);
void mon_handle_disconnect(tcpcon* con);
void cleanup();
void handle_exit(int sig);

int main()
{
    signal(SIGINT, handle_exit);

    if (init())
    {
        tcpserv_start(serv_airplanes);
        tcpserv_start(serv_cmd);
        tcpserv_start(serv_mon);
    }
    else
    {
        perror("Bind failed");
        cleanup();
        return -1;
    }

    while (keepalive)
    {
        pthread_mutex_lock(&mutex);
            detect_collisions();
            broadcast();
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    cleanup();

    return 0;
}

int init()
{
    airplanes = list_create(5);
    cmd_consoles = list_create(5);
    mon_consoles = list_create(5);

    serv_airplanes = tcpserv_create(CTRL_HOST, PORT_AP, ap_handle_connect, ap_handle_message, ap_handle_disconnect);
    serv_cmd = tcpserv_create(CTRL_HOST, PORT_CMD, NULL, NULL, NULL);
    serv_mon = tcpserv_create(CTRL_HOST, PORT_MON, mon_handle_connect, NULL, mon_handle_disconnect);

    return serv_airplanes && serv_cmd && serv_mon;
}

char* serialize_airplane_list()
{
    static char srl_buffer[4000];
    static char srl_buffer_staging[1000];

    memset(srl_buffer, 0, sizeof(srl_buffer));
    memset(srl_buffer_staging, 0, sizeof(srl_buffer_staging));

    pair_t* pair;
    long len;

    for (unsigned int i = 0; i < airplanes->size; i++)
    {
        pair = (pair_t*)list_get(airplanes, i);

        airplane_serialize(pair->second, srl_buffer_staging, &len);
        char* bufferEnc = base64_encode(srl_buffer_staging, len, &len);
        strncat(srl_buffer, bufferEnc, (unsigned long)len);

        if (i + 1 < airplanes->size)
            strcat(srl_buffer, ";");

        free(bufferEnc);
    }

    char* result = (char*)calloc(4010, sizeof(char));
    strcat(result, "lst::<");
    strcat(result, srl_buffer);
    strcat(result, ">");

    return result;
}

void broadcast()
{
    char* str_aplist = serialize_airplane_list();
    msg_t* msg = msg_create(HINT_AIRPLANE_LIST, str_aplist, NULL, NULL);
    free(str_aplist);

    tcpcon* con;
    for (unsigned int i = 0; i < mon_consoles->size; i++)
    {
        con = list_get(mon_consoles, i);
        tcpcon_sendmsg(con, msg);
    }

    msg_free(msg);
}

void detect_collisions()
{
    pair_t* pair;
    airplane *target, *other;

    for (unsigned int i = 0; i < airplanes->size; i++)
    {
        pair = (pair_t*)list_get(airplanes, i);
        collision_detect_clear((airplane*)pair->second);
    }

    for (unsigned int i = 0; i < airplanes->size; i++)
    {
        target = (airplane*)((pair_t*)list_get(airplanes, i))->second;

        for (unsigned int j = 0; j < airplanes->size; j++)
        {
            if (i == j) continue;

            other = (airplane*)((pair_t*)list_get(airplanes, j))->second;
            collision_detect_mark(target, other);
        }
    }
}

airplane* find_airplane(int socket)
{
    pair_t* p;
    for (unsigned int i = 0; i < airplanes->size; i++)
    {
        p = ((pair_t*)list_get(airplanes, i));
        if (socket == *(int*)p->first)
            return (airplane*)p->second;
    }
    return NULL;
}

void add_airplane(int socket, airplane* ap)
{
    int* id = (int*)calloc(1, sizeof(int));
    *id = socket;
    pair_t* pair = pair_create(id, ap);
    list_push(airplanes, pair);
}

void remove_airplane(int socket)
{
    pair_t* p;
    for (unsigned int i = 0; i < airplanes->size; i++)
    {
        p = ((pair_t*)list_get(airplanes, i));
        if (socket == *(int*)p->first)
        {
            list_removeat(airplanes, i);
            free(p->first);
            free(p->second);
            free(p);
            break;
        }
    }
}

void ap_handle_connect(tcpcon* con)
{
    puts("Ap connected");
}

void ap_handle_message(tcpcon* con, char* msg)
{
    if (!keepalive)
        return;

    pthread_mutex_lock(&mutex);
        if (msg_isvalid(msg))
        {
            msg_t* m = msg_deserialize(msg);

            airplane* ap = (airplane*)calloc(1, sizeof(airplane));
            airplane_deserialize(m->body, ap);

            airplane* existing_ap = find_airplane(con->sockfd);

            if (existing_ap)
            {
                memcpy(existing_ap, ap, sizeof(airplane));
                free(ap);
            }
            else
            {
                add_airplane(con->sockfd, ap);
            }
        }
    pthread_mutex_unlock(&mutex);
}

void ap_handle_disconnect(tcpcon* con)
{
    if (!keepalive)
        return;

    pthread_mutex_lock(&mutex);
        puts("Ap disconnected");
        remove_airplane(con->sockfd);
    pthread_mutex_unlock(&mutex);
}

void mon_handle_connect(tcpcon* con)
{
    if (!keepalive)
        return;

    pthread_mutex_lock(&mutex);
        list_push(mon_consoles, con);
        puts("mon connected");
    pthread_mutex_unlock(&mutex);
}

void mon_handle_disconnect(tcpcon* con)
{
    if (!keepalive)
        return;

    pthread_mutex_lock(&mutex);
        list_remove(mon_consoles, con);
        puts("mon disconnected");
    pthread_mutex_unlock(&mutex);
}

void cleanup()
{
    pthread_mutex_lock(&mutex);
        if (airplanes)
            list_free(airplanes, YES);

        if (cmd_consoles)
            list_free(cmd_consoles, YES);

        if (mon_consoles)
            list_free(mon_consoles, YES);

        if (serv_airplanes)
        {
            tcpserv_stop(serv_airplanes);
            tcpserv_free(serv_airplanes);
            serv_airplanes = NULL;
        }

        if (serv_cmd)
        {
            tcpserv_stop(serv_cmd);
            tcpserv_free(serv_cmd);
            serv_cmd = NULL;
        }

        if (serv_mon)
        {
            tcpserv_stop(serv_mon);
            tcpserv_free(serv_mon);
            serv_mon = NULL;
        }
    pthread_mutex_unlock(&mutex);
}

void handle_exit(int sig)
{
    keepalive = 0;
    puts("Exiting");
}
