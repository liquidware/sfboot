/* System Calls Remapping */
#include <LPC23xx.H>                       /* LPC23xx definitions  */
#include <stdint.h>
#include <stdio.h>                         /* standard I/O .h-file */
#include <math.h>

#include <stdlib.h>
#include <math.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <string.h>
#include <errno.h>

//#define putchar sendchar
//#define getchar getkey

//extern int putchar (int ch);
//extern int getchar (void);



int _read (int fd, char * ptr, int len) {

  return 0;
}

int _lseek (int fd, int ptr, int dir) {
  return (0);
}

int _write (int fd, char * ptr, int len) {
  return (0);
}

int _open (const char * path, int flags, ...) {
  return (-1);
}

int _close (int fd) {
  return (-1);
}

int _kill (int n, int m) {
  return(-1);
}

int __attribute__((weak)) _getpid (int n __attribute__ ((unused))) {
  return (1);
}

extern int  __cs3_heap_start;
extern int  __cs3_heap_end;

caddr_t _sbrk (int incr) {


  return 0;
}

int __attribute__((weak)) _fstat (int fd, struct stat * st) {

  return (0);
}

int __attribute__((weak))_link (void) {
  return (-1);
}

int _unlink (const char *path) {
  return (-1);
}

int isatty (int fd) {
  return (1);
}

//void __exidx_start(void){ return 0; }

extern void __exidx_start(void){  }

extern void __exidx_end(void){  }

extern void abort(void) {
while(1);
 }

extern void _init(void) {

}
//extern void memcpy(void) {
// }

//extern void restore_core_regs(void) {
// }



#ifdef _USE_TIME

int _gettimeofday (struct timeval * tp, void * tzvp) {

  return (0);
}

extern long timeval;

/* Return a clock that ticks at 100Hz.  */
clock_t _times (struct tms * tp) {

return 0;
};

#endif
