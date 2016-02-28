#ifndef __AJP_H
#define __AJP_H

#include <sock.h>
#include <stdlib.h>
#include <joedog/boolean.h>

#define AJP13_FORWARD_REQUEST 2
#define AJP13_SEND_CHUNK      3
#define AJP13_SEND_HEADERS    4
#define AJP13_END_RESPONSE    5
#define AJP13_GET_CHUNK       6
#define AJP13_CPONG           9
#define AJP13_CPING          10
#define AJP13_TERMINATOR   0xff

typedef struct AJP13_T *AJP13;

extern size_t  AJP13SIZE;

AJP13    new_ajp13();
BOOLEAN  ajp13_ping(AJP13 this, SOCK sock);
BOOLEAN  ajp13_pong(AJP13 this, SOCK sock);
int      ajp13_sent(AJP13 this);
int      ajp13_recv(AJP13 this);

#endif/*__AJP_H*/
