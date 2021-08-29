#ifndef __MESSAGE_OBJECT_H__
#define __MESSAGE_OBJECT_H__

#include <sys/types.h>

typedef struct
{
    u_int32_t type;
    char data[1024];
} messageObject;

#define MSG_PING 1
#define MSG_PONG 2
#define MSG_TYPE1 11
#define MSG_TYPE2 21

#endif //! __MESSAGE_OBJECT_H__