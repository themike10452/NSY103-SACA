#ifndef MESSAGE_H
#define MESSAGE_H

#define HINT_COMMAND                   1 << 0
#define HINT_LOCK                      1 << 1
#define HINT_RELEASE                   1 << 2
#define HINT_ALERT                     1 << 3
#define HINT_LOCK_ACK                  1 << 4
#define HINT_RELEASE_ACK               1 << 5
#define HINT_AIRPLANE_LIST             1 << 10
#define HINT_ID_LIST                   1 << 11

#define MSG_MAX_LENGTH 4000

typedef struct msg_t msg_t;

struct msg_t
{
    unsigned int hints;
    char* body;
    char* from;
    char* to;
};

msg_t* msg_create(unsigned int hints, char* body, char* from, char* to);
void msg_free(msg_t* msg);
char* msg_serialize(const msg_t* msg);
msg_t* msg_deserialize(const char* msg);
unsigned int msg_isvalid(const char* msg);

#endif
