#include "../common/utils/list.h"
#include "airplane.h"

#ifndef COLLISIONDETECTION_H
#define COLLISIONDETECTION_H

void collision_detect_mark(airplane* target, const airplane* other);
void collision_detect_clear(airplane* ap);

#endif
