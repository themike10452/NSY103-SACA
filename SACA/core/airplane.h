#ifndef AIRPLANE_H
#define AIRPLANE_H

#include "../common/math/vec3.h"

#define AP_FLAG_WARN 0x1
#define AP_FLAG_DNGR 0x2
#define AP_FLAG_DAMG 0x3

#define AP_FLAG_HIGHLIGHTED 0x4

#define AP_MASK_CD 0x03
#define AP_MASK_DISP 0x04

typedef struct airplane airplane;

struct airplane {
    vec3 coords;
    vec3 dir;
    float roll;
    float pitch;
    float yaw;
    float speed;
    char id[6];
    unsigned int flags;
};

void airplane_serialize(const airplane* ap, char* out_str, long int* out_len);
void airplane_deserialize(const char* str, airplane* out_ap);

#endif
