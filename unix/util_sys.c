#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "util_sys.h"

unsigned long util_sys_seconds()
{
  return (unsigned long)time(0);
}

unsigned long long util_sys_ms(unsigned long epoch)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if(epoch > (unsigned long)tv.tv_sec) return 0;
  return (unsigned long long)(tv.tv_sec - epoch) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}

unsigned short util_sys_short(unsigned short x)
{
  return ntohs(x);
}

void util_sys_random_init(void)
{
  struct timeval tv;
  unsigned int seed;

  // TODO ifdef for srandomdev when avail
  gettimeofday(&tv, NULL);
  seed = (getpid() << 16) ^ tv.tv_sec ^ tv.tv_usec;
  srandom(seed);
}

long util_sys_random(void)
{
  // TODO, use ifdef for arc4random
  return random();
}

#ifdef DEBUG
int _logging = 1;
#else
int _logging = 0;
#endif

void util_sys_logging(int enabled)
{
  if(enabled < 0)
  {
    _logging ^= 1;
  }else{
    _logging = enabled;    
  }
  LOG("log output enabled");
}

void *util_sys_log(const char *file, int line, const char *function, const char * format, ...)
{
  char buffer[256];
  va_list args;
  if(!_logging) return NULL;
  va_start (args, format);
  vsnprintf (buffer, 256, format, args);
  fprintf(stderr,"%s:%d %s() %s\n", file, line, function, buffer);
  fflush(stderr);
  va_end (args);
  return NULL;
}
