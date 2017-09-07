#include "airplane.h"
#include "controller.h"
#include "../common/net/tcpcon.h"
#include "../common/math/rotator.h"

#include <unistd.h>
#include <signal.h>

struct airplane ap;
tcpcon* con = NULL;
int keepalive = 1;

void init();
void update();
int open_connection();
int close_connection();
void send_updates();
void handle_message(tcpcon* c, char* msg);
void handle_disconnect(tcpcon* c);
void handle_exit(int sig);

int main(int argc, char* argv[])
{
    signal(SIGINT, handle_exit);

    init();

    if (argc > 1) // test scenarios
    {
        if (strcmp(argv[1], "11") == 0)
        {
            ap.coords.x = 58.232407f;
            ap.coords.y = 13.146552f;
            ap.coords.z = 0;
            ap.dir.x = 1;
            ap.dir.y = 0;
            ap.dir.z = 0;
            ap.speed = 485.0f;
        }
        else if (strcmp(argv[1], "12") == 0)
        {
            ap.coords.x = 66.832169f;
            ap.coords.y = 20.215082f;
            ap.coords.z = 0;
            ap.dir.x = 0.207909f;
            ap.dir.y = -0.978148f;
            ap.dir.z = 0;
            ap.speed = 505.0f;
        }
        else if (strcmp(argv[1], "21") == 0)
        {
            ap.coords.x = 69.456604f;
            ap.coords.y = 21.405430f;
            ap.coords.z = 0;
            ap.dir.x = 0.255443f;
            ap.dir.y = -0.966824f;
            ap.dir.z = 0;
            ap.speed = 926.0f;
        }
        else if (strcmp(argv[1], "22") == 0)
        {
            ap.coords.x = 63.967148f;
            ap.coords.y = 16.379311f;
            ap.coords.z = 0;
            ap.dir.x = 1;
            ap.dir.y = 0;
            ap.dir.z = 0;
            ap.speed = 926.0f;
        }
        else if (strcmp(argv[1], "31") == 0)
        {
            ap.coords.x = 10;
            ap.coords.y = 10;
            ap.yaw = 0;
            ap.speed = 100;
        }
        else if (strcmp(argv[1], "32") == 0)
        {
            ap.coords.x = 11;
            ap.coords.y = 11;
            ap.yaw = 90;
            ap.speed = 100;
        }
    } // .test scenarios

    int success;
    do
    {
        success = open_connection();
        if (!success)
        {
            perror("Failed to communicate with SACA Controller");
            puts("Retrying in 5 ...");
            sleep(5);
        }
    }
    while (!success && keepalive);

    while (keepalive)
    {
        update();
        send_updates();

        sleep(1);
    }

    close_connection();
}

void init()
{
    unsigned int seed = (unsigned int)time(NULL);
    srandom(seed);

    memset(&ap, 0, sizeof(ap));

    sprintf(ap.id, "AP-%d%d", (int)(random() % 10), (int)(random() % 10));
}

void update()
{
    static const vec3 initial_dir = { 1, 0, 0 };
    static struct timespec last_tick = {0, 0};
    static struct timespec now;
    static long delta;
    static rotator rot;

    if (last_tick.tv_sec == 0)
    {
        clock_gettime(CLOCK_MONOTONIC_RAW, &last_tick);
        return;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    delta = ((now.tv_sec - last_tick.tv_sec) * 1000000 + (now.tv_nsec - last_tick.tv_nsec)) / 1000;

    rot.x = ap.roll;
    rot.y = ap.pitch;
    rot.z = ap.yaw;

    vec3* new_dir = rotator_rotvec3(&rot, &initial_dir);
    memcpy(&ap.dir, new_dir, sizeof(vec3));

    vec3* movement = vec3_fmultiply(new_dir, (ap.speed / 3600) * (delta / 1000.0f));
    vec3_applyadd(&ap.coords, movement);

    free(new_dir);
    free(movement);

    clock_gettime(CLOCK_MONOTONIC_RAW, &last_tick);
}

void send_updates()
{
    static char buffer[1000];
    static long len;
    airplane_serialize(&ap, buffer, &len);
    msg_t* m = msg_create(0, buffer, NULL, NULL);
    tcpcon_sendmsg(con, m);
    msg_free(m);
}

int open_connection()
{
    con = tcpcon_open(CTRL_HOST, PORT_AP, handle_message, handle_disconnect);
    return con != NULL;
}

int close_connection()
{
    tcpcon_close(con);
    tcpcon_free(con);
}

void handle_message(tcpcon* c, char* msg)
{
    if (msg_isvalid(msg))
    {
        msg_t* m = msg_deserialize(msg);
        if (m->hints & HINT_COMMAND)
        {
            printf("%d: %s\n", c->sockfd, m->body);
        }
        msg_free(m);
    }
}

void handle_disconnect(tcpcon* c)
{
    puts("Disconnected");
}

void handle_exit(int sig)
{
    keepalive = 0;
    puts("Exiting");
}
