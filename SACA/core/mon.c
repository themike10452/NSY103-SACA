#include "controller.h"
#include "airplane.h"
#include "../common/net/tcpcon.h"
#include "../common/utils/base64.h"

#include <unistd.h>
#include <signal.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

list_t* deserialize_airplane_list(const char* str);
void handle_message(tcpcon* c, char* msg);
void handle_exit(int sig);

tcpcon* con;
int keepalive = 1;

int main()
{
    signal(SIGINT, handle_exit);

    do
    {
        con = tcpcon_open(CTRL_HOST, PORT_MON, handle_message, NULL);
        if (!con)
        {
            perror("Failed to communicate with SACA Controller");
            puts("Retrying in 5 ...");
            sleep(5);
        }
    }
    while (!con && keepalive);

    while (keepalive)
    {
        sleep(1);
    }

    if (con)
    {
        tcpcon_close(con);
        tcpcon_free(con);
    }

    return 0;
}

list_t* deserialize_airplane_list(const char* str)
{
    static unsigned int count;

    char* strw = str_unwrap(str);
    char** elements = str_split(strw, ';', &count);

    list_t* list = list_create(count);

    char* element;
    for (int i = 0; i < count; i++)
    {
        element = elements[i];
        long len = strlen(element);
        if (len > 0)
        {
            char* apstr = base64_decode(element, len, NULL);
            if (apstr)
            {
                airplane* ap = (airplane*)malloc(sizeof(airplane));
                airplane_deserialize(apstr, ap);

                if (ap)
                    list_push(list, ap);

                free(apstr);
            }
        }
    }

    str_freearray(elements, count);
    free(strw);

    return list;
}

void handle_message(tcpcon* c, char* msg)
{
    static const char* underline = "---------------------------------------";

    char* output_color;

    if (msg_isvalid(msg))
    {
        msg_t* m = msg_deserialize(msg);

        if (m->hints & HINT_AIRPLANE_LIST)
        {
            printf("\x1b[2J\x1b[H");
            printf("%6s %16s %8s %5s %7s\n", "Id", "Coordinates", "Altitude", "Pitch", "Speed");
            printf("%.*s %.*s %.*s %.*s %.*s\n", 6, underline, 16, underline, 8, underline, 5, underline, 7, underline);

            if (strlen(m->body))
            {
                list_t* airplanes = deserialize_airplane_list(m->body);

                airplane* ap;
                for (unsigned int i = 0; i < airplanes->size; i++)
                {
                    ap = list_get(airplanes, i);

                    int cd_state = ap->flags & AP_MASK_CD;

                    if (cd_state == AP_FLAG_WARN)
                        output_color = KYEL;
                    else if (cd_state == AP_FLAG_DNGR)
                        output_color = KRED;
                    else if (cd_state == AP_FLAG_DAMG)
                        output_color = KMAG;
                    else
                        output_color = KNRM;

                    printf("%s%6s  %7.1f,%7.1f %8.1f %5.1f %7.2f\n" KNRM, output_color, ap->id, ap->coords.x, ap->coords.y, ap->coords.z, ap->pitch, ap->speed);
                }

                list_free(airplanes, YES);
            }

            fflush(stdout);
        }

        msg_free(m);
    }
}

void handle_exit(int sig)
{
    keepalive = 0;
}
