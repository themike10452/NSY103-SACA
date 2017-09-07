#include "../common/net/tcpcon.h"
#include "controller.h"
#include "../common/utils/stringutils.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

tcpcon* con;
list_t* aplist;
char apid[6] = {0};
int keepalive = 1;
int lockacquired = 0;

pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_id_list = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_lock_resp = PTHREAD_COND_INITIALIZER;

void handle_message(tcpcon* c, char* msg);
void handle_disconnect(tcpcon* con);
void handle_exit(int sig);

int main()
{
    signal(SIGINT, handle_exit);

    do
    {
        con = tcpcon_open(CTRL_HOST, PORT_CMD, handle_message, handle_disconnect);
        if (!con)
        {
            perror("Failed to communicate with SACA Controller");
            puts("Retrying in 5 ...");
            sleep(5);
        }
    }
    while (!con && keepalive);

    static char input[256] = {0};

    pthread_mutex_lock(&wait_mutex);
        pthread_cond_wait(&cond_id_list, &wait_mutex);
    pthread_mutex_unlock(&wait_mutex);

    printf("~>");
    scanf("%s", input);

    puts("Please wait ...");

    msg_t* lockmsg = msg_create(HINT_LOCK, NULL, NULL, input);
    tcpcon_sendmsg(con, lockmsg);
    free(lockmsg);

    pthread_mutex_lock(&wait_mutex);
        pthread_cond_wait(&cond_lock_resp, &wait_mutex);
    pthread_mutex_unlock(&wait_mutex);

    if (lockacquired)
    {
        printf("\x1b[2J\x1b[H");
        while (keepalive)
        {
            printf("%s>", apid);
            memset(input, 0, sizeof(input));

            fgets(input, sizeof(input), stdin);

            if (strcmp(input, "exit\n") == 0)
            {
                keepalive = 0;
                break;
            }

            msg_t* msg = msg_create(HINT_COMMAND, input, NULL, apid);
            tcpcon_sendmsg(con, msg);
            msg_free(msg);
            puts("");
        }
    }
    else
    {
        puts("Connection rejected. Please try again later.");
        keepalive = 0;
    }

    if (con)
    {
        tcpcon_close(con);
        tcpcon_free(con);
    }

    if (aplist)
        list_free(aplist, YES);
}

void handle_message(tcpcon* c, char* msg)
{
    if (!keepalive)
        return;

    if (msg_isvalid(msg))
    {
        msg_t* m = msg_deserialize(msg);

        pthread_mutex_lock(&wait_mutex);

            if (!aplist && m->hints & HINT_ID_LIST)
            {
                unsigned int count;
                char** ids = str_split(m->body, ';', &count);

                aplist = list_create(count);

                for (int i = 0; i < count; i++)
                {
                    list_push(aplist, ids[i]);
                    puts(ids[i]);
                }

                free(ids);

                pthread_cond_signal(&cond_id_list);
            }
            else if (m->hints & HINT_LOCK_ACK)
            {
                lockacquired = (strcmp(m->body, "OK") == 0);

                if (lockacquired)
                    str_cpy(apid, m->to, strlen(m->to));
                else
                    memset(apid, 0, sizeof(apid));

                pthread_cond_signal(&cond_lock_resp);
            }

        pthread_mutex_unlock(&wait_mutex);
    }
}

void handle_disconnect(tcpcon* con)
{
    if (!keepalive)
        return;

    puts("\nDisconnected");
}

void handle_exit(int sig)
{
    keepalive = 0;
}
