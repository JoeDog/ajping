#include <ajp.h>
#include <sock.h>
#include <notify.h>
#include <stdint.h>
#include <string.h>

struct AJP13_T
{
  int          sent;
  int          recv;
  uint8_t      ping[5];
  uint8_t      pong[5];
};

size_t AJP13SIZE = sizeof(struct AJP13_T);

AJP13
new_ajp13()
{
  AJP13 this;
  this = calloc(AJP13SIZE, 1);
  return this;
}


BOOLEAN 
ajp13_ping(AJP13 this, SOCK sock)
{
  int bytes     = 0;
  this->ping[0] = 0x12;
  this->ping[1] = 0x34;
  this->ping[2] = 0;
  this->ping[3] = 1;
  this->ping[4] = AJP13_CPING;
  
  bytes = socket_write(sock, this->ping, 5);
  if (bytes == 5) {
    this->sent += 1;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN
ajp13_pong(AJP13 this, SOCK sock)
{
  int bytes = 0;

  memset(this->pong, 0, sizeof(this->pong));

  bytes = socket_read(sock, &this->pong, 5); 
  if (bytes == 5) {
    if ((this->pong[0] == 'A') && (this->pong[1] == 'B') && (this->pong[2] == 0) && 
        (this->pong[3] ==   1) && (this->pong[4] == AJP13_CPONG)) {
      this->recv += 1;
      return TRUE;
    }
    // we got five bytes but WTF?
    NOTIFY (
      ERROR, "CPONG: data corruption: %02x,%02x,%02x,%02x,%02x",
      this->pong[0], this->pong[1], this->pong[2], this->pong[3], this->pong[4]
    );
  }
  NOTIFY(ERROR, "CPONG: unable to read the server response");
  return FALSE; 
}

int
ajp13_sent(AJP13 this)
{
  return this->sent;
}

int 
ajp13_recv(AJP13 this)
{
  return this->recv;
}
