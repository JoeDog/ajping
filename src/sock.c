#ifdef  HAVE_CONFIG_H
#include <config.h>
#endif/*HAVE_CONFIG_H*/
#include <setup.h>
#include <stdlib.h>
#include <stdio.h>
#include <sock.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_POLL
# include <poll.h>
#endif/*HAVE_POLL*/
#include <notify.h>


struct SOCK_T
{
  int      sock;
  char *   host;
  char *   addr;
  int      port;  
  int      timo;
  char     rbuf[4096];
  size_t   rlen;
  int      rpos;
  fd_set * ws;
  fd_set * rs;
  SDSET    state;
  S_STATUS status;
#ifdef  HAVE_POLL
  struct pollfd pfd[1];
#endif/*HAVE_POLL*/
};

size_t SOCKSIZE = sizeof(struct SOCK_T);

private BOOLEAN __tcpv4_connect(SOCK this);
private BOOLEAN __tcpv6_connect(SOCK this);
private ssize_t __socket_write(int sock, const void *vbuf, size_t len);
private int     __socket_block(SOCK this, BOOLEAN block);
private BOOLEAN __socket_check(SOCK this, SDSET mode);
#ifdef  HAVE_POLL
private BOOLEAN __socket_poll(SOCK this, SDSET mode);
#endif/*HAVE_POLL*/
private BOOLEAN __socket_select(SOCK this, SDSET mode);



SOCK
new_socket(char *host, int port)
{
  SOCK    this;
  BOOLEAN res = FALSE;
  this = calloc(SOCKSIZE, 1);
  this->host = strdup(host);
  this->port = port; 
  if (my.ipv6) {
    res = __tcpv6_connect(this);
  } else {
    res = __tcpv4_connect(this);
  }
  if (res == FALSE) {
    this = socket_destroy(this);
    return this;
  }
  return this;
}

SOCK
socket_destroy(SOCK this) 
{
  if (this != NULL) {
    free(this->host);
    free(this);
    this = NULL;
  }
  return this;
}

/**
 * returns void
 * socket_write wrapper function.
 */
int
socket_write(SOCK this, const void *buf, size_t len)
{
  size_t bytes;

  if ((bytes = __socket_write(this->sock, buf, len)) != len) {
    NOTIFY(ERROR, "unable to write to socket %s:%d", __FILE__, __LINE__);
    return -1;
  }

  return bytes;
}

ssize_t
socket_read(SOCK this, void *vbuf, size_t len)
{
  size_t      n;
  ssize_t     r;
  char       *buf;
  int ret_eof = 0;

  buf = vbuf;
  n   = len;

  while (n > 0) {
    if (this->rlen < len) {
      if (__socket_check(this, READ) == FALSE) {
        NOTIFY(WARNING, "socket: read check timed out(%d) %s:%d", (my.timeout)?my.timeout:15, __FILE__, __LINE__);
        return -1;
      }
    }
    if (this->rlen <  n) {
      int lidos;
      memmove(this->rbuf,&this->rbuf[this->rpos],this->rlen);
      this->rpos = 0;
      if (__socket_check(this, READ) == FALSE) {
        NOTIFY(WARNING, "socket: read check timed out(%d) %s:%d", (my.timeout)?my.timeout:15, __FILE__, __LINE__);
        return -1;
      }
      lidos = read(this->sock, &this->rbuf[this->rlen], sizeof(this->rbuf)-this->rlen);
      if (lidos == 0)
        ret_eof = 1;
      if (lidos < 0) {
        if (errno==EINTR || errno==EAGAIN)
          lidos = 0;
        if (errno==EPIPE){
          return 0;
        } else {
          NOTIFY(ERROR, "socket: read error %s %s:%d", strerror(errno), __FILE__, __LINE__);
          return 0; /* was return -1 */
        }
      }
      this->rlen += lidos;
    }
    if (this->rlen >= n) {
      r = n;
    } else {
      r = this->rlen;
    }
    if (r == 0) break;
    memmove(buf,&this->rbuf[this->rpos],r);
    this->rpos  += r;
    this->rlen  -= r;
    n   -= r;
    buf += r;
    if (ret_eof) break;
  } /* end of while */
  return (len - n);
}

char *
socket_address(SOCK this)
{
  if (this->addr != NULL && strlen(this->addr) > 2) {
    return this->addr;
  }
  return NULL;
}


void
socket_close(SOCK this)
{
  if (this==NULL) return;

  if (this->sock != -1) {
    /*if ((__socket_block(this->sock, FALSE)) < 0) {
      NOTIFY(ERROR, "Unable to set to non-blocking %s:%d", __FILE__, __LINE__);
    }*/
    if (shutdown(this->sock, 2) < 0) {
      NOTIFY(ERROR, "Unable to shutdown the socket %s:%d", __FILE__, __LINE__);
    }
    if (close(this->sock) < 0) {
      NOTIFY(ERROR, "Unable to close the socket %s:%d",    __FILE__, __LINE__);
    }
  }
  this->sock = -1;
  return;
}

private ssize_t
__socket_write(int sock, const void *vbuf, size_t len)
{
  size_t      n;
  ssize_t     w;
  const char *buf;

  buf = vbuf;
  n   = len;
  while (n > 0) {
    if ((w = write( sock, buf, n)) <= 0) {
      if (errno == EINTR) {
        w = 0;
      } else {
        return -1;
      }
    }
    n   -= w;
    buf += w;
  }
  return len;
}

/**
 * Conditionally determines whether or not a socket is ready.
 * This function calls __socket_poll if HAVE_POLL is defined in
 * config.h, else it uses __socket_select
 */
private BOOLEAN
__socket_check(SOCK this, SDSET mode)
{
#ifdef HAVE_POLL
 if (this->sock >= FD_SETSIZE) {
   return __socket_poll(this, mode);
 } else {
   return __socket_select(this, mode);
 }
#else
 return __socket_select(this, mode);
#endif/*HAVE_POLL*/
}

#ifdef HAVE_POLL
private BOOLEAN
__socket_poll(SOCK this, SDSET mode)
{
  int res;
  int timo = (my.timeout) ? my.timeout * 1000 : 15000;
  __socket_block(this, FALSE);

  this->pfd[0].fd      = this->sock + 1;
  this->pfd[0].events |= POLLIN;

  do {
    res = poll(this->pfd, 1, timo);
    if (res < 0) puts("LESS THAN ZERO!");
  } while (res < 0); // && errno == EINTR);

  if (res == 0) {
    errno = ETIMEDOUT;
  }

  if (res <= 0) {
    this->state = UNDEF;
    NOTIFY(ERROR,
      "socket: polled(%d) and discovered it's not ready %s:%d",
      (my.timeout)?my.timeout:15, __FILE__, __LINE__
    );
    return FALSE;
  } else {
    this->state = mode;
    return TRUE;
  }
}
#endif/*HAVE_POLL*/

private BOOLEAN
__socket_select(SOCK this, SDSET mode)
{
  struct timeval timeout;
  int    res;
  fd_set rs;
  fd_set ws;
  memset((void *)&timeout, '\0', sizeof(struct timeval));
  timeout.tv_sec  = (my.timeout > 0)?my.timeout:30;
  timeout.tv_usec = 0;

  if ((this->sock < 0) || (this->sock >= FD_SETSIZE)) {
    // FD_SET can't handle it
    return FALSE;
  }

  do {
    FD_ZERO(&rs);
    FD_ZERO(&ws);
    FD_SET(this->sock, &rs);
    FD_SET(this->sock, &ws);
    res = select(this->sock+1, &rs, &ws, NULL, &timeout);
  } while (res < 0 && errno == EINTR);

  if (res == 0) {
    errno = ETIMEDOUT;
  }

  if (res <= 0) {
    this->state = UNDEF;
    NOTIFY(ERROR, "socket: select and discovered it's not ready %s:%d", __FILE__, __LINE__);
    return FALSE;
  } else {
    this->state = mode;
    return TRUE;
  }
}

/**
 * local function
 * set socket to non-blocking
 */
private int
__socket_block(SOCK this, BOOLEAN block)
{
#if HAVE_FCNTL_H
  int flags;
  int retval;
#elif defined(FIONBIO)
  ioctl_t status;
#else
  return this->sock;
#endif
return this->sock;
  if (this->sock==-1) {
    return this->sock;
  }

#if HAVE_FCNTL_H
  if ((flags = fcntl(this->sock, F_GETFL, 0)) < 0) {
    switch (errno) {
      case EACCES: {NOTIFY(ERROR, "EACCES %s:%d",                 __FILE__, __LINE__); break;}
      case EBADF:  {NOTIFY(ERROR, "bad file descriptor %s:%d",    __FILE__, __LINE__); break;}
      case EAGAIN: {NOTIFY(ERROR, "address is unavailable %s:%d", __FILE__, __LINE__); break;}
      default:     {NOTIFY(ERROR, "unknown network error %s:%d",  __FILE__, __LINE__); break;}
    } return -1;
  }

  if (block) {
    flags &= ~O_NDELAY;
  } else {
    flags |=  O_NDELAY;
    #if (defined(hpux) || defined(__hpux) || defined(__osf__)) || defined(__sun)
    #else
    flags |=  O_NONBLOCK;
    #endif
  }

  if ((retval = fcntl(this->sock, F_SETFL, flags)) < 0) {
    NOTIFY(ERROR, "unable to set fcntl flags %s:%d", __FILE__, __LINE__);
    return -1;
  }
  return retval;

#elif defined(FIONBIO)
  status = block ? 0 : 1;
  return ioctl(this->sock, FIONBIO, &status);
#endif
}

private BOOLEAN 
__tcpv4_connect(SOCK this) 
{
  int    res = 0;
  int    herrno;
  struct sockaddr_in cli;
  struct hostent     *hp;
  int    conn;
#if defined(__GLIBC__)
  struct hostent hent;
  char   hbf[8192];
#endif/*__GLIBC__*/

  if ((this->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    switch (errno) {
      case EPROTONOSUPPORT: { NOTIFY(ERROR, "Unsupported protocol %s:%d",  __FILE__, __LINE__); break; }
      case EMFILE:          { NOTIFY(ERROR, "Descriptor table full %s:%d", __FILE__, __LINE__); break; }
      case ENFILE:          { NOTIFY(ERROR, "File table full %s:%d",       __FILE__, __LINE__); break; }
      case EACCES:          { NOTIFY(ERROR, "Permission denied %s:%d",     __FILE__, __LINE__); break; }
      case ENOBUFS:         { NOTIFY(ERROR, "Insufficient buffer %s:%d",   __FILE__, __LINE__); break; }
      default:              { NOTIFY(ERROR, "Unknown socket error %s:%d",  __FILE__, __LINE__); break; }
    } socket_close(this); return FALSE; 
  }

  if (fcntl(this->sock, F_SETFD, O_NDELAY) < 0) {
    NOTIFY(ERROR, "Unable to set close control %s:%d", __FILE__, __LINE__);
  }

#if defined(__GLIBC__)
  {
    memset(hbf, '\0', sizeof hbf);
    /* for systems using GNU libc */
    if ((gethostbyname_r(this->host, &hent, hbf, sizeof(hbf), &hp, &herrno) < 0)) {
      hp = NULL;
    }
  }
#else
  {
    hp = gethostbyname(hn);
    herrno = h_errno;
  }
#endif/*__GLIBC__*/
 
  /**
   * If hp is NULL, then we did not get good information
   * from the name server. Let's notify the user and bail
   */
  if (hp == NULL) {
    switch (herrno) {
      case HOST_NOT_FOUND: { NOTIFY(ERROR, "Host not found: %s\n", this->host);                           break; }
      case NO_ADDRESS:     { NOTIFY(ERROR, "Host does not have an IP address: %s\n", this->host);         break; }
      case NO_RECOVERY:    { NOTIFY(ERROR, "A non-recoverable resolution error for %s\n", this->host);    break; }
      case TRY_AGAIN:      { NOTIFY(ERROR, "A temporary resolution error for %s\n", this->host);          break; }
      default:             { NOTIFY(ERROR, "Unknown error code from gethostbyname for %s\n", this->host); break; }
    }
    return FALSE;
  }
  memset((void*) &cli, 0, sizeof(cli));
  memcpy(&cli.sin_addr, hp->h_addr, hp->h_length);  
  cli.sin_family = AF_INET;
  cli.sin_port   = htons(this->port);
  this->addr     = strdup(inet_ntoa(cli.sin_addr));

  if ((__socket_block(this, FALSE)) < 0) {
    NOTIFY(ERROR, "socket: unable to set socket to non-blocking %s:%d", __FILE__, __LINE__);
    return FALSE;
  }

  /**
   * connect to the host
   * evaluate the server response and check for
   * readability/writeability of the socket....
   */
  conn = connect(this->sock, (struct sockaddr *)&cli, sizeof(struct sockaddr_in));
  if (conn < 0 && errno != EINPROGRESS) {
    switch (errno) {
      case EACCES:        {NOTIFY(ERROR, "Socket: access denied"          ); break;}
      case EADDRNOTAVAIL: {NOTIFY(ERROR, "Socket: address is unavailable."); break;}
      case ETIMEDOUT:     {NOTIFY(ERROR, "Socket: connection timed out."  ); break;}
      case ECONNREFUSED:  {NOTIFY(ERROR, "Socket: connection refused."    ); break;}
      case ENETUNREACH:   {NOTIFY(ERROR, "Socket: network is unreachable."); break;}
      case EISCONN:       {NOTIFY(ERROR, "Socket: already connected."     ); break;}
      default:            {NOTIFY(ERROR, "Socket: unknown network error." ); break;}
    } socket_close(this); return FALSE;
  } else {
    if (__socket_check(this, READ) == FALSE) {
      NOTIFY(ERROR, "Socket: read check timed out(%d) %s:%d", my.timeout, __FILE__, __LINE__);
      socket_close(this);
      return FALSE;
    } else {
      /**
       * If we reconnect and receive EISCONN, then we have a successful connection
       */
      res = connect(this->sock, (struct sockaddr *)&cli, sizeof(struct sockaddr_in));
      if((res < 0)&&(errno != EISCONN)){
        NOTIFY(ERROR, "Socket: unable to connect %s:%d", __FILE__, __LINE__);
        socket_close(this);
        return FALSE;
      }
      this->status = S_READING;
    }
  } /* end of connect conditional */

  if ((__socket_block(this, TRUE)) < 0) {
    NOTIFY(ERROR, "Socket: unable to set socket to non-blocking %s:%d", __FILE__, __LINE__);
    return FALSE;
  }
  return (this->sock > 0);
}

private BOOLEAN 
__tcpv6_connect(SOCK this)
{
  int    res;
  int    conn;
  char   tmp[100];
  struct sockaddr_in6 cli;
  struct hostent *hp;

  this->sock = socket(AF_INET6, SOCK_STREAM, 0);
  if ((this->sock = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    switch (errno) {
      case EPROTONOSUPPORT: { NOTIFY(ERROR, "Unsupported protocol %s:%d",  __FILE__, __LINE__); break; }
      case EMFILE:          { NOTIFY(ERROR, "Descriptor table full %s:%d", __FILE__, __LINE__); break; }
      case ENFILE:          { NOTIFY(ERROR, "File table full %s:%d",       __FILE__, __LINE__); break; }
      case EACCES:          { NOTIFY(ERROR, "Permission denied %s:%d",     __FILE__, __LINE__); break; }
      case ENOBUFS:         { NOTIFY(ERROR, "Insufficient buffer %s:%d",   __FILE__, __LINE__); break; }
      default:              { NOTIFY(ERROR, "Unknown socket error %s:%d",  __FILE__, __LINE__); break; }
    } socket_close(this); return FALSE;
  }
 
  hp = gethostbyname2(this->host,AF_INET6);
  if (hp == NULL) {
    switch (h_errno) {
      case HOST_NOT_FOUND: { NOTIFY(ERROR, "Host not found: %s\n", this->host);                           break; }
      case NO_ADDRESS:     { NOTIFY(ERROR, "Host does not have an IP address: %s\n", this->host);         break; }
      case NO_RECOVERY:    { NOTIFY(ERROR, "A non-recoverable resolution error for %s\n", this->host);    break; }
      case TRY_AGAIN:      { NOTIFY(ERROR, "A temporary resolution error for %s\n", this->host);          break; }
      default:             { NOTIFY(ERROR, "Unknown error code from gethostbyname for %s\n", this->host); break; }
    } socket_close(this); return FALSE;
  }

  memset((void*) &cli, 0, sizeof(cli));
  memcpy(&cli.sin6_addr, hp->h_addr, hp->h_length);
  cli.sin6_flowinfo = 0;
  cli.sin6_family   = AF_INET6;
  cli.sin6_port     = htons(this->port);
  memset(tmp, '\0', 100);
  this->addr        = strdup(inet_ntop(AF_INET6, &(cli.sin6_addr), tmp, 100)); 

  if ((__socket_block(this, FALSE)) < 0) {
    NOTIFY(ERROR, "socket: unable to set socket to non-blocking %s:%d", __FILE__, __LINE__);
    return FALSE;
  }

  conn = connect(this->sock, (struct sockaddr *)&cli, sizeof(struct sockaddr_in6));
  if (conn < 0 && errno != EINPROGRESS) {
    switch (errno) {
      case EACCES:        {NOTIFY(ERROR, "Socket: access denied"          ); break;}
      case EADDRNOTAVAIL: {NOTIFY(ERROR, "Socket: address is unavailable."); break;}
      case ETIMEDOUT:     {NOTIFY(ERROR, "Socket: connection timed out."  ); break;}
      case ECONNREFUSED:  {NOTIFY(ERROR, "Socket: connection refused."    ); break;}
      case ENETUNREACH:   {NOTIFY(ERROR, "Socket: network is unreachable."); break;}
      case EISCONN:       {NOTIFY(ERROR, "Socket: already connected."     ); break;}
      default:            {NOTIFY(ERROR, "Socket: unknown network error." ); break;}
    } socket_close(this); return FALSE;
  } else {
    if (__socket_check(this, READ) == FALSE) {
      NOTIFY(ERROR, "Socket: read check timed out(%d) %s:%d", my.timeout, __FILE__, __LINE__);
      socket_close(this);
      return FALSE;
    } else {
      /**
       * If we reconnect and receive EISCONN, then we have a successful connection
       */
      res = connect(this->sock, (struct sockaddr *)&cli, sizeof(struct sockaddr_in6));
      if((res < 0)&&(errno != EISCONN)){
        NOTIFY(ERROR, "Socket: unable to connect %s:%d", __FILE__, __LINE__);
        socket_close(this);
        return FALSE;
      }
      this->status = S_READING;
    }
  } /* end of connect conditional */

  if ((__socket_block(this, TRUE)) < 0) {
    NOTIFY(ERROR, "Socket: unable to set socket to non-blocking %s:%d", __FILE__, __LINE__);
    return FALSE;
  }
  return (this->sock > 0);
}
