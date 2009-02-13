/*                                              -*- mode:C -*-
 *
 * SFBChecksum.c - Cryptographically INSECURE, but pretty fast
 *                 and small LFSR-based checksum
 *
 * Copyright (C) 2008-2009 David H. Ackley.  All rights reserved.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

/* TO TEST THIS CODE:
 * - If in the SFB tree, e.g.:
 *   gcc -DTEST_CHECKSUM -I../../include SFBChecksum.c;./a.out
 *
 * - If standalone: Put SFBChecksum.cpp and SFBChecksum.h in same dir, then:
 *   gcc -DTEST_CHECKSUM SFBChecksum.c;./a.out
 *
 * Either way, result should be 'All tests passed' with zero exit status.
 */

#include "SFBChecksum.h"

void SFBChecksumInit(SFBChecksum cs) {
  cs[0] = 0x5FB00B00;        /* SFBooboo - Make an unused checksum noticeable.. */
  cs[1] = 0x0DBA62A9;        /* plus one from /dev/urandom, now forever enshrined */
}

void SFBChecksumAddByte(SFBChecksum cs, char data) {
  int shift_in;

  cs[1] ^= data;                  /* Inject data at bottom */
 
  shift_in = ((~cs[0]>>30)^(cs[0]>>29))&1;  /* == bit 63 xnor bit 62 */

  cs[0] = (cs[0]<<1)|(cs[1]>>31); /* Top of low into bottom of high */
  cs[1] = (cs[1]<<1)|shift_in;    /* And bring around the carry */
}

void SFBChecksumAddBytes(SFBChecksum cs, const char * data, int length) {
  int i;
  for (i = 0; i < length; ++i)
    SFBChecksumAddByte(cs,*data++);
}

int SFBChecksumEqual(const SFBChecksum cs1,const SFBChecksum cs2) {
  return cs1[0] == cs2[0] && cs1[1] == cs2[1];
}

static char buf[2*8+1];    /* Max usage is 2 words of 8 hex chars plus null byte */

const char * SFBChecksumToString(SFBChecksum cs) {
  int out = 0;
  int i;
  for (i = 0; i < 2; ++i) {
    int j;
    for (j = 28; j >= 0;  j -= 4) {
      buf[out++] = "0123456789abcdef"[(cs[i]>>j)&0xf];
    }
  }
  buf[out++] = 0;
  return buf;
}

int SFBChecksumFromString(SFBChecksum cs, const char * hexString) {
  int out = 0;
  int i;
  for (i = 0; i < 2; ++i) {
    int j;
    cs[i] = 0;
    for (j = 28; j >= 0;  j -= 4) {
      int val;
      char ch = hexString[out++];
      if (ch >= '0' && ch <= '9') val = ch-'0';
      else if (ch >= 'a' && ch <= 'f') val = ch-'a'+10;
      else if (ch >= 'A' && ch <= 'F') val = ch-'A'+10;
      else return 0;
      cs[i] |= val<<j;
    }
  }
  return 1;
}

#ifdef TEST_CHECKSUM
#include <stdio.h>  /* for printf */
#include <string.h> /* for strlen, strcmp */
#include <stdlib.h> /* for exit, srandom, random */

const char * SFBChecksumString(SFBChecksum cs, const char * zstring) {
  SFBChecksumInit(cs);
  SFBChecksumAddBytes(cs, zstring,strlen(zstring));
  return SFBChecksumToString(cs);
}

static SFBChecksum s;
static void checkResult(const char * answer, const char * input) {
  SFBChecksum cs;
  if (!SFBChecksumFromString(cs,answer)) exit(2); 

  if (!SFBChecksumEqual(s,cs)) {
    printf("expected %s, but got %s", answer, SFBChecksumToString(s));
    if (input)
      printf(" on '%s', ",input);
    printf("\n");
    exit(1);
  }
}

static void testBytes(const char * data, int length, const char * answer) {
  SFBChecksumInit(s);
  SFBChecksumAddBytes(s,data,length);
  checkResult(answer, (const char *) data);
}

static void testString(const char * string, const char * result) {
  testBytes(string, strlen(string), result);
}

static void testRepeats(const char ch, int count, const char * answer) {
  int i;
  SFBChecksumInit(s);
  for (i = 0; i < count; ++i)
    SFBChecksumAddByte(s,ch);
  checkResult(answer,0);
}

static int testBuriedChange(const int count, const int seed, const int incr, const int randomChars) {
  int i;
  int distanceIn = count/2;
  SFBChecksum s1, s2;

  SFBChecksumInit(s1);
  SFBChecksumInit(s2);
  srandom(seed);
  for (i = 0; i < count; ++i) {
    char ch = i;
    if (randomChars) ch = random()&0xff; 

    SFBChecksumAddByte(s1,ch);
    if (i == distanceIn) {          /* At the halfway point.. */
      if (incr) ch++;
      else                  
        ch ^= 1<<(random()&0x7);      /* ..flip one bit */
    }
    SFBChecksumAddByte(s2,ch);
  }
  return SFBChecksumEqual(s1,s2);
}

#define RANDOM_STRING_SIZE 512
static char buf1[RANDOM_STRING_SIZE];
static char buf2[RANDOM_STRING_SIZE];

static int testRandomStrings(const int seed, const int nchanges) {
  int i;
  SFBChecksum s1, s2;

  srandom(seed);
  do {
    for (i = 0; i < RANDOM_STRING_SIZE; ++i) {
      buf1[i] = random()&0xff;
      buf2[i] = buf1[i];
    }
    for (i = 0; i < nchanges; ++i) {
      int pos = random()%RANDOM_STRING_SIZE;
      int bit = 1<<(random()&0x7);
      buf2[pos] ^= bit;
    }
  } while (!strncmp(buf1,buf2,RANDOM_STRING_SIZE));
  
  SFBChecksumInit(s1);
  
  for (i = 0; i < RANDOM_STRING_SIZE; ++i) {
    SFBChecksumAddByte(s1,buf1[i]);
    SFBChecksumAddByte(s2,buf2[i]);
  }

  return SFBChecksumEqual(s1,s2);
}

static void testShouldCollide(const char * p1, int l1, const char * p2, int l2) {
  SFBChecksum s1, s2;
  SFBChecksumInit(s1);
  SFBChecksumInit(s2);
  SFBChecksumAddBytes(s1,p1,l1);
  SFBChecksumAddBytes(s2,p2,l2);
  if (!SFBChecksumEqual(s1,s2)) {
      printf("Constructed sums failed to collide (%s)\n",SFBChecksumToString(s1));
      exit(1);
  }
} 

int main() {

  testString("abc","fd8058006dd3170f");
  testString("acb","fd8058006dd31709");
  testString("cab","fd8058006dd31711");
  testString("cba","fd8058006dd3171b");
  testString("bca","fd8058006dd31717");
  testString("bac","fd8058006dd3171b");

  testString("Hello World","8058006dd316188a");
  testString("hello world","8058006dd3171c8a");
  testString("hello world.","00b000dba62e3949");
  testString("ABCDEFGHIJKLMNOPQRSTUVWXYZ","0036e98b5b890ff7");
  testString("abcdefghijklmnopqrstuvwxyz","0036e98ba476f037");
  testString("0bcdefghijklmnopqrstuvwxyz","0036e98ae076f037");
  testString("1bcdefghijklmnopqrstuvwxyz","0036e98ae476f037");
  testString("2bcdefghijklmnopqrstuvwxyz","0036e98ae876f037");
  testString("3bcdefghijklmnopqrstuvwxyz","0036e98aec76f037");
  testString("4bcdefghijklmnopqrstuvwxyz","0036e98af076f037");

  testString("","5fb00b000dba62a9");
  testString("5fb00b000dba62a9fec549d7aad8d993","0dba62b11bb01b75");
  testString("74826c5d067638cee1b3e8b298cbdc1e","0dba628c398ced29");
  testString("de3b36630e8e389ba96a040efd5a6fbf","0dba62fd02d11b8b");
  testString("653b186e76a494f80921a8d3b37ed79e","0dba6287cd67bec9");

  testRepeats('a',  1,"bf6016001b74c590");
  testRepeats('a',  2,"7ec02c0036e98be2");
  testRepeats('a',  5,"f6016001b74c5d59");
  testRepeats('a', 10,"c02c0036e98ba347");
  testRepeats('a', 20,"b000dba62e8c1a42");
  testRepeats('a', 50,"8ba30680e800b634");
  testRepeats('a',100,"d9d4e7285c09c04f");
  testRepeats('a',200,"8348026fa1dee20a");
  testRepeats('a',500,"c6fbb0b0db7d88e3");

  testRepeats('\0',1<<0, "bf6016001b74c552");
  testRepeats('\0',1<<1, "7ec02c0036e98aa4");
  testRepeats('\0',1<<2, "fb00b000dba62a93");
  testRepeats('\0',1<<3, "b00b000dba62a93e");
  testRepeats('\0',1<<4, "0b000dba62a93e5f");
  testRepeats('\0',1<<5, "0dba62a93e5fc5ff");
  testRepeats('\0',1<<6, "3e5fc5ffd262b009");
  testRepeats('\0',1<<7, "7a3f63ff12b05fc8");
  testRepeats('\0',1<<8, "b4f44ff29e3b7c52");
  testRepeats('\0',1<<9, "44f4f2482734689e");
  testRepeats('\0',1<<10,"f94590eca309045f");
  testRepeats('\0',1<<11,"cc1a58a913c28a09");
  testRepeats('\0',1<<12,"bb9f59ff3f2d103e");
  testRepeats('\0',1<<13,"3134d3f27374dc65");
  testRepeats('\0',1<<14,"0fff424546f0eb33");
  testRepeats('\0',1<<15,"424e9d5b7bc2933e");
  testRepeats('\0',1<<16,"caa037ba4f3471a9");
  testRepeats('\0',1<<17,"887afea9d31065c8");
  testRepeats('\0',1<<18,"755475f2b3a633a4");
  testRepeats('\0',1<<19,"c1346e48ca7bc8a9");
  testRepeats('\0',1<<20,"b24e20e1c2cd87f2");

  {
    int i;
    const int cases = 2000;
    int aliases = 0;
    for (i = 1; i < cases; ++i) {
      if (testBuriedChange(1000,i,0,1)) ++aliases;
      if (testBuriedChange(1000,i,0,0)) ++aliases;
      if (testBuriedChange(i,i,1,1)) ++aliases;
    }
    if (aliases != 0) {
      printf("Small buried change went undetected in %d of %d tests\n",aliases,3*cases);
      exit(1);
    }
  }
  {
    int i, j;
    const int cases = 500;
    for (j = 1; j <= 32; ++j) {
      for (i = 1; i <= cases; ++i) {
        if (testRandomStrings(i,j)) {
          printf("Random strings differing in no more than %d bits collided\n",j);        
          exit(1);
        }
      }
    }
  }

  testShouldCollide("\1\0",2,"\0\2",2);      /* Since LFSRs don't avalanche.. */
  testShouldCollide("\3\6",2,"\2\4",2);      /* ..it's easy to hand-build collisions. */
  testShouldCollide("2d!'N9r",7,"(P!)R,X",7);/* Ah well.  Price of small and fast. */

  printf("All tests passed\n");
  return 0;
}
#endif
