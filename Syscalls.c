/*****************************************************************************
   Illuminato  X Machina - Secondary Bootloader
   Copyright (C) 2009 Christopher Ladden

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
   USA
 ******************************************************************************/
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

extern void __exidx_start(void){  }

extern void __exidx_end(void){  }

extern void abort(void) {
while(1);
 }

extern void _init(void) {

}

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
