#include "message.h"
#include "../utils/stringutils.h"
#include "../utils/base64.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

msg_t* msg_create(unsigned int hints, char* body, char* from, char* to)
{
    int len;
    msg_t* msg = (msg_t*)calloc(1, sizeof(msg_t));
    msg->hints = hints;

    if (body)
    {
        len = strlen(body);
        msg->body = (char*)malloc(len + 1);
        str_cpy(msg->body, body, len);
    }
    else
    {
        msg->body = (char*)calloc(1, sizeof(char));
    }

    if (from)
    {
        len = strlen(from);
        msg->from = (char*)malloc(len + 1);
        str_cpy(msg->from, from, len);
    }
    else
    {
        msg->from = (char*)calloc(1, sizeof(char));
    }

    if (to)
    {
        len = strlen(to);
        msg->to = (char*)malloc(len + 1);
        str_cpy(msg->to, to, len);
    }
    else
    {
        msg->to = (char*)calloc(1, sizeof(char));
    }

    return msg;
}

void msg_free(msg_t* msg)
{
    if (msg)
    {
        if (msg->body)
        {
            free(msg->body);
            msg->body = NULL;
        }

        if (msg->from)
        {
            free(msg->from);
            msg->from = NULL;
        }

        if (msg->to)
        {
            free(msg->to);
            msg->to = NULL;
        }

        free(msg);
    }
}

char* msg_serialize(const msg_t* msg)
{
    char* msg_str = (char*)malloc(4000);
    snprintf
    (
        msg_str,
        MSG_MAX_LENGTH, 
        "msg::<%d;%s;%s;%s>",
        msg->hints,
        base64_encode(msg->from, strlen(msg->from), NULL),
        base64_encode(msg->to, strlen(msg->to), NULL),
        base64_encode(msg->body, strlen(msg->body), NULL)
    );
    return msg_str;
}

msg_t* msg_deserialize(const char* msg)
{
    unsigned int size;
    char* str = str_unwrap(msg);
    char** parts = str_split(str, ';', &size);
    
    msg_t* m = msg_create
    (
        atoi(parts[0]),
        base64_decode(parts[3], strlen(parts[3]), NULL),
        base64_decode(parts[1], strlen(parts[1]), NULL),
        base64_decode(parts[2], strlen(parts[2]), NULL)
    );

    str_freearray(parts, size);
    free(str);
    return m;
}

unsigned int msg_isvalid(const char* msg)
{
    return str_startswith(msg, "msg::");
}
