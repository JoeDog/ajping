#ifdef  HAVE_CONFIG_H
# include <config.h>
#endif/*HAVE_CONFIG_H*/
#define  INTERN  1
#include <setup.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ajp.h>
#include <url.h>
#include <notify.h>
#include <sock.h>
#include <util.h>
#include <version.h>
#include <sys/time.h>

#ifdef HAVE_LIMITS_H
# include <limits.h>
#else 
# define INT_MAX 2147483647
#endif/*HAVE_LIMITS_H*/

#ifdef  __CYGWIN__
# include <getopt.h>
#else
# include <joedog/getopt.h>
#endif/*__CYGWIN__*/

private void   __usage();
private void   __version(BOOLEAN quit);

/**
 * long options, std options struct
 */
private struct option long_options[] =
{
  {"version",      no_argument,       NULL, 'V'},
  {"help",         no_argument,       NULL, 'h'},
  {"verbose",      no_argument,       NULL, 'v'},
  {"quiet",        no_argument,       NULL, 'q'},
  {"debug",        no_argument,       NULL, 'D'},
  {"reps",         required_argument, NULL, 'r'},
  {"time",         required_argument, NULL, 't'},
  {"ip",           required_argument, NULL, 'i'},
  {0, 0, 0, 0}
};

private void
__version(BOOLEAN quit)
{
  /**
   * version_string is defined in version.c
   * adding it to a separate file allows us
   * to parse it in configure.
   */
  char name[128];

  memset(name, 0, sizeof name);
  strncpy(name, program_name, strlen(program_name));

  if (quit) {
    fprintf(stderr,"%s %s\n\n%s\n", uppercase(name, strlen(name)), version_string, copyright);
    exit(0);
  } else {
    fprintf(stderr,"%s %s\n", uppercase(name, strlen(name)), version_string);
  }
}

private void
__usage()
{
  __version(FALSE);
  printf("Usage: %s URL\n",           program_name);
  printf("       %s [options] URL\n", program_name);
  printf("       %s [options]\n",     program_name);
  printf("Options:\n");
  puts("  -V, --version             VERSION, prints the version number.");
  puts("  -h, --help                HELP, prints this section.");
  puts("  -v, --verbose             VERBOSE, prints notification to screen.");
  puts("  -q, --quiet               QUIET turns verbose off and suppresses output.");
  puts("  -r, --reps=NUM            REPS, number of times to run the test." );
  puts("  -t, --time=NUMm           TIMED testing where \"m\" is modifier S, M, or H");
  puts("  -i, --ip=NUM              IP version. Choices 4 or 6 (default is 4)");
  puts("");
  puts(copyright);
  /**
   * our work is done, exit nicely
   */
  exit( EXIT_SUCCESS );
}



private URL
__parse_cmdline(int argc, char *argv[])
{
  int c = 0;
  int n = 0;
  int nargs;
  URL url    = NULL;
  my.reps    = 0;
  my.ipv6    = FALSE;
  my.timeout = 5;
  my.loop    = TRUE;
  my.quiet   = FALSE;

  while ((c = getopt_long(argc, argv, "Vhvqdr:t:i:", long_options, (int *)0)) != EOF) {
    switch (c) {
      case 'V':
        __version(TRUE);
        break;
      case 'h':
        __usage();
        exit(0);
      case 'r':
        my.reps = atoi(optarg);
        break;
      case 't':
        my.timeout = atoi(optarg);
        break;
      case 'q':
        my.quiet = TRUE;
        break;
      case 'i':
        n = atoi(optarg);
        if (n == 6) my.ipv6 = TRUE;
        break;
    }
  }   /* end of while c = getopt_long */
  nargs = argc - optind;
  if (nargs) {
    url = new_url(argv[argc-1]);
  }
  if (url == NULL) {
    __usage();
  }
  return url;
}

#pragma GCC diagnostic ignored "-Wformat-zero-length"
private BOOLEAN
__ajp_ping(URL U)
{
  char   addr[256];
  int    sent  = 0;
  int    recv  = 0;
  int    min   = INT_MAX;
  int    max   = -1;
  double loss  = 0;
  int    count = 0;
  int    limit = (my.reps == 0) ? INT_MAX : my.reps;
  AJP13  ajp   = new_ajp13();
  SOCK   sock  = new_socket(url_get_hostname(U), url_get_port(U));
  struct timeval stop, start;
  unsigned long int sum = 0;

  if (sock == NULL) {
    return FALSE;
  }

  VERBOSE(my.quiet, "--- %s v%s to %s:%d ---", program_name, version_string, url_get_hostname(U), url_get_port(U));

  memset(addr, '\0', sizeof(addr));
  if (socket_address(sock) != NULL) {
    snprintf(addr, sizeof(addr), " (%s)", socket_address(sock));
  } else {
    snprintf(addr, sizeof(addr), "");
  }
 
  while (my.loop) {
    (void)gettimeofday(&start, NULL);
    if (ajp13_ping(ajp, sock) == FALSE) {
      return FALSE;
    } 
    if (ajp13_pong(ajp, sock) == FALSE) {
      return FALSE;
    }
    (void)gettimeofday(&stop, NULL);

    VERBOSE (my.quiet,
      " 5 bytes from %s%s: seq=%d time=%lu ms", 
      url_get_hostname(U), addr, count+1, stop.tv_usec - start.tv_usec
    );
    sum += (stop.tv_usec - start.tv_usec);
    min  = (stop.tv_usec - start.tv_usec < min) ? stop.tv_usec - start.tv_usec : min;
    max  = (stop.tv_usec - start.tv_usec > max) ? stop.tv_usec - start.tv_usec : max;
    count++;
    sleep(1);
    if (count >= limit) break;
  }
  sent = ajp13_sent(ajp);
  recv = ajp13_recv(ajp);
  if (sent > 1) {
    loss = (sent == recv) ? 0.00 : (sent / recv) * 100;
  } else {
    loss = 100;
  }
  VERBOSE(my.quiet, "\n--- %s:%d ajping statistics ---", url_get_hostname(U), url_get_port(U));
  VERBOSE(my.quiet, "%d packets sent, %d received, %0.f%% packet loss, time: %lu ms", sent, recv, loss, sum);
  VERBOSE(my.quiet, "rtt min/avg/max = %d/%lu/%d ms", min, (sum/count), max);
  return TRUE;
}
#pragma GCC diagnostic warning "-Wformat-zero-length"

void sig_handler(int signo)
{
  switch (signo) {
    default: 
      my.loop = FALSE;
      break;
  }
}

int
main(int argc, char *argv[])
{
  URL     url = NULL;
  BOOLEAN res = FALSE;
  struct sigaction sa;
  sa.sa_handler = &sig_handler;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);
  sigaction(SIGKILL, &sa, NULL); 
  sigaction(SIGINT, &sa, NULL);

  url  =  __parse_cmdline(argc, argv);
  res  = __ajp_ping(url);
 
  if (res == FALSE) {
    exit(1);
  } 
  exit(0); 
}

