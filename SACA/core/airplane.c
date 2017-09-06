#include "airplane.h"
#include "../common/utils/base64.h"

#include <stdlib.h>

void airplane_serialize(const airplane* ap, char* out_str, long int* out_len)
{
    int len;
    
    char* result = (char*)calloc(1, 1000 * sizeof(char));
    char* coord_str = vec3_serialize(&ap->coords);
    char* dir_str = vec3_serialize(&ap->dir);

    char* coord_str_enc = base64_encode(coord_str, strlen(coord_str), NULL);
    char* dir_str_enc = base64_encode(dir_str, strlen(dir_str), NULL);

    sprintf
    (
        result,
        "ap::<%s;%s;%s;%f;%f;%f;%f;%d>",
        ap->id,         //0
        coord_str_enc,  //1
        dir_str_enc,    //2
        ap->roll,       //3
        ap->pitch,      //4
        ap->yaw,        //5
        ap->speed,      //6
        ap->flags       //7
    );

    len = strlen(result);

    str_cpy(out_str, result, len);

    if (out_len)
        *out_len = len;

    free(result);
    free(coord_str);
    free(dir_str);
    free(coord_str_enc);
    free(dir_str_enc);
}

void airplane_deserialize(const char* str, airplane* out_ap)
{
    if (!str || !strlen(str)) return;

    unsigned int count;
    char* body = str_unwrap(str);
    char** parts = str_split(body, ';', &count);

    out_ap->roll = (float)atof(parts[3]);
    out_ap->pitch = (float)atof(parts[4]);
    out_ap->yaw = (float)atof(parts[5]);
    out_ap->speed = (float)atof(parts[6]);
    out_ap->flags = (unsigned int)atoi(parts[7]);

    char* coords_str_dec = base64_decode(parts[1], strlen(parts[1]), NULL);
    char* dir_str_dec = base64_decode(parts[2], strlen(parts[2]), NULL);

    vec3* coords = vec3_deserialize(coords_str_dec);
    vec3* dir = vec3_deserialize(dir_str_dec);

    str_cpy(out_ap->id, parts[0], strlen(parts[0]));
    vec3_copy(coords, &out_ap->coords);
    vec3_copy(dir, &out_ap->dir);

    free(body);
    free(coords_str_dec);
    free(dir_str_dec);
    free(coords);
    free(dir);
    str_freearray(parts, count);
}
