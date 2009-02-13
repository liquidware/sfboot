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

#ifndef SFBCHECKSUM_H
#define SFBCHECKSUM_H

#include <inttypes.h>  /* for uint32_t */

/**
 * An SFBChecksum is a fairly fast, fairly small, ARM friendly, 64 bit
 * checksum generator, based on a (63 bit) linear feedback shift
 * register.  It is designed to gain reasonable confidence that a
 * sequence of data bytes has not been corrupted during transmission.
 *
 * NOTE WELL: SFBChecksum is _ONLY_ aimed at detecting changes due to
 * noise or bugs, and it is _NOT_ cryptographically secure!
 *
 * PAY ATTENTION: Matching SFBChecksum's is _not_ a defense against
 * malicious intent!
 *
 * The SFB protocol uses SFBChecksum's to check that completed
 * received images match what is expected.
 *
 * Internally, an 'SFBChecksum' is simply an array of two 32 bit
 * unsigned ints.
 */

typedef uint32_t SFBChecksum[2];

/**
 * Initialize an SFBChecksum struct.  May be called at any time, may
 * be called repeatedly, must be called before the SFBChecksum is
 * first used and whenever a new SFBChecksum is being started.
 */
void SFBChecksumInit(SFBChecksum cs);

/**
 * Incorporate an additional data byte in an SFBChecksum struct.
 *
 * @param ws Pointer to the structure to initialize.  Must be non-NULL
 *
 * @param data Byte to incorporate into the running SFBChecksum.
 */
void SFBChecksumAddByte(SFBChecksum cs, char data);

/**
 * Incorporate a sequence of additional data bytes into an SFBChecksum
 * struct.  
 *
 * @param data Pointer to an array of 'length' bytes to add to the
 *             running SFBChecksum in order, starting with data[0] and 
 *             including up through data[length-1].  Must be non-NULL
 *
 * @param length Number of bytes in array 'data' to use.  If length is
 *               less than or equal to zero, the SFBChecksum remains 
 *               unchanged.
 *
 */
void SFBChecksumAddBytes(SFBChecksum cs, const char * data, int length);

/**
 * Copy one checksum to another.
 *
 * @param csFrom source
 *
 * @param csTo destination
 */
void SFBChecksumCopy(const SFBChecksum csFrom,SFBChecksum csTo);

/**
 * Compare one checksum to another.
 *
 * @param cs1 one checksum
 *
 * @param cs2 the other checksum
 *
 * @return 1 iff cs1==cs2, 0 otherwise
 * 
 */
int SFBChecksumEqual(const SFBChecksum cs1,const SFBChecksum cs2);


/**
 * Access the current SFBChecksum result, as a zero-terminated, 16
 * character hexadecimal string.  Note this function returns a pointer
 * to a statically allocated buffer, so:
 *
 * (1) You MUST NOT 'free' this result;
 *
 * (2) The content pointed-to by this result is only valid until the
 *     next call of SFBChecksumToString _on ANY_ SFBChecksum struct
 *
 * Note the pointed-to value is 16 ASCII-encoded hexadecimal characters.
 *
 * @param cs Checksum to read out
 *
 * @return Pointer to a zero-terminated string of 16 hexadecimal
 *         characters containing the current SFBChecksum result.  See
 *         limitations above on the use of the pointed-to-data.  Never
 *         returns NULL.
 *
 */
const char * SFBChecksumToString(SFBChecksum cs);

/**
 * Initialize an SFBChecksum from a string of 16 ASCII hex characers.
 *
 * @param cs Checksum to initialize
 *
 * @param hexString Pointer to an array of at least 16 ASCII hex 
 *         characters with nothing else (no leading or embedded 
 *         whitespace, etc).  Routine only looks at first 16 positions,
 *         so anything after that is not a problem.
 *
 * @return 1 if cs was successfully initialized from hexString, 
 *         0 an invalid (non-hex) ASCII character was encountered.
 *
 */
int SFBChecksumFromString(SFBChecksum cs, const char * hexString);

#endif /* SFBCHECKSUM_H */
