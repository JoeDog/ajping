#ifndef __SOCK_H
#define __SOCK_H

#include <stdlib.h>

typedef enum
{
  UNDEF = 0,
  READ  = 1,
  WRITE = 2,
  RDWR  = 3
} SDSET;

typedef enum
{
  S_CONNECTING = 1,
  S_READING    = 2,
  S_WRITING    = 4,
  S_DONE       = 8
} S_STATUS;

/**
 * a URL object
 */
typedef struct SOCK_T *SOCK;

/**
 * For memory allocation; SOCKSIZE
 * provides the object size
 */
extern size_t  SOCKSIZE;

SOCK     new_socket(char *host, int port);
SOCK     socket_destroy(SOCK this);
void     socket_close(SOCK this);
ssize_t  socket_read(SOCK this, void *vbuf, size_t len);
int      socket_write(SOCK this, const void *buf, size_t len);
char *   socket_address(SOCK this);

#endif/*__SOCK_H*/
