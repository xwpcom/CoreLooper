/*
 *  HAVEGE: HArdware Volatile Entropy Gathering and Expansion
 *
 *  Copyright (C) 2006-2009, Paul Bakker <polarssl_maintainer at polarssl.org>
 *  All rights reserved.
 *
 *  Joined copyright on original XySSL code with: Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *  The HAVEGE RNG was designed by Andre Seznec in 2002.
 *
 *  http://www.irisa.fr/caps/projects/hipsor/publi.php
 *
 *  Contact: seznec(at)irisa_dot_fr - orocheco(at)irisa_dot_fr
 */

#include "stdafx.h"
#ifdef _SUPPORT_SSL
#include <string.h>
#include <time.h>

#include "config.h"

#if defined(POLARSSL_HAVEGE_C)

#include "havege.h"
#include "timing.h"
using namespace Bear::Core;
/* ------------------------------------------------------------------------
 * On average, one iteration accesses two 8-word blocks in the havege WALK
 * table, and generates 16 words in the RES array.
 *
 * The data read in the WALK table is updated and permuted after each use.
 * The result of the hardware clock counter read is used  for this update.
 *
 * 25 conditional tests are present.  The conditional tests are grouped in
 * two nested  groups of 12 conditional tests and 1 test that controls the
 * permutation; on average, there should be 6 tests executed and 3 of them
 * should be mispredicted.
 * ------------------------------------------------------------------------
 */

unsigned long hardclock_EX()
{
#ifdef _MSC_VER
	return GetTickCount();
#else
	static int idx=0;
	idx++;
	return idx;
#endif
}


#define SWAP(X,Y) { int *T = X; X = Y; Y = T; }

#define TST1_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;
#define TST2_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;

#define TST1_LEAVE U1++; }
#define TST2_LEAVE U2++; }

#define ONE_ITERATION                                   \
                                                        \
    PTEST = PT1 >> 20;                                  \
                                                        \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
                                                        \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
                                                        \
    PTX = (PT1 >> 18) & 7;                              \
    PT1 &= 0x1FFF;                                      \
    PT2 &= 0x1FFF;                                      \
    CLK = (int) hardclock_EX();                            \
                                                        \
    i = 0;                                              \
    A = &WALK[PT1    ]; RES[i++] ^= *A;                 \
    B = &WALK[PT2    ]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 1]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 4]; RES[i++] ^= *D;                 \
                                                        \
    IN_ = (*A >> (1)) ^ (*A << (31)) ^ CLK;              \
    *A = (*B >> (2)) ^ (*B << (30)) ^ CLK;              \
    *B = IN_ ^ U1;                                       \
    *C = (*C >> (3)) ^ (*C << (29)) ^ CLK;              \
    *D = (*D >> (4)) ^ (*D << (28)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 2]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 2]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 3]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 6]; RES[i++] ^= *D;                 \
                                                        \
    if( PTEST & 1 ) SWAP( A, C );                       \
                                                        \
    IN_ = (*A >> (5)) ^ (*A << (27)) ^ CLK;              \
    *A = (*B >> (6)) ^ (*B << (26)) ^ CLK;              \
    *B = IN_; CLK = (int) hardclock_EX();                   \
    *C = (*C >> (7)) ^ (*C << (25)) ^ CLK;              \
    *D = (*D >> (8)) ^ (*D << (24)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 4];                                 \
    B = &WALK[PT2 ^ 1];                                 \
                                                        \
    PTEST = PT2 >> 1;                                   \
                                                        \
    PT2 = (RES[(i - 8) ^ PTY] ^ WALK[PT2 ^ PTY ^ 7]);   \
    PT2 = ((PT2 & 0x1FFF) & (~8)) ^ ((PT1 ^ 8) & 0x8);  \
    PTY = (PT2 >> 10) & 7;                              \
                                                        \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
                                                        \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
                                                        \
    C = &WALK[PT1 ^ 5];                                 \
    D = &WALK[PT2 ^ 5];                                 \
                                                        \
    RES[i++] ^= *A;                                     \
    RES[i++] ^= *B;                                     \
    RES[i++] ^= *C;                                     \
    RES[i++] ^= *D;                                     \
                                                        \
    IN_ = (*A >> ( 9)) ^ (*A << (23)) ^ CLK;             \
    *A = (*B >> (10)) ^ (*B << (22)) ^ CLK;             \
    *B = IN_ ^ U2;                                       \
    *C = (*C >> (11)) ^ (*C << (21)) ^ CLK;             \
    *D = (*D >> (12)) ^ (*D << (20)) ^ CLK;             \
                                                        \
    A = &WALK[PT1 ^ 6]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 3]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 7]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 7]; RES[i++] ^= *D;                 \
                                                        \
    IN_ = (*A >> (13)) ^ (*A << (19)) ^ CLK;             \
    *A = (*B >> (14)) ^ (*B << (18)) ^ CLK;             \
    *B = IN_;                                            \
    *C = (*C >> (15)) ^ (*C << (17)) ^ CLK;             \
    *D = (*D >> (16)) ^ (*D << (16)) ^ CLK;             \
                                                        \
    PT1 = ( RES[(i - 8) ^ PTX] ^                        \
            WALK[PT1 ^ PTX ^ 7] ) & (~1);               \
    PT1 ^= (PT2 ^ 0x10) & 0x10;                         \
                                                        \
    for( n++, i = 0; i < 16; i++ )                      \
        hs->pool[n % COLLECT_SIZE] ^= RES[i];

/*
 * Entropy gathering function
 */
static void havege_fill( havege_state *hs )
{
    int i, n = 0;
    int  U1,  U2, *A, *B, *C, *D;
    int PT1, PT2, *WALK, RES[16];
    int PTX, PTY, CLK, PTEST, IN_;

    WALK = hs->WALK;
    PT1  = hs->PT1;
    PT2  = hs->PT2;

    PTX  = U1 = 0;
    PTY  = U2 = 0;

    memset( RES, 0, sizeof( RES ) );

    while( n < COLLECT_SIZE * 4 )
    {
        ONE_ITERATION
        ONE_ITERATION
        ONE_ITERATION
        ONE_ITERATION
    }

    hs->PT1 = PT1;
    hs->PT2 = PT2;

    hs->offset[0] = 0;
    hs->offset[1] = COLLECT_SIZE / 2;
}

/*
 * HAVEGE initialization
 */
void havege_init_EX( havege_state *hs )
{
    memset( hs, 0, sizeof( havege_state ) );

    havege_fill( hs );
}

/*
 * HAVEGE rand function
 */
int havege_rand_EX( void *p_rng )
{
    int ret=0;
    havege_state *hs = (havege_state *) p_rng;

    if( hs->offset[1] >= COLLECT_SIZE )
        havege_fill( hs );

    if(hs->offset[0]>=0 && hs->offset[0]<COLLECT_SIZE
		&& hs->offset[1]>=0 && hs->offset[1]<COLLECT_SIZE
		)
	{
		ret  = hs->pool[hs->offset[0]++];
		ret ^= hs->pool[hs->offset[1]++];
	}
	else
	{
		DW("hs->offset[0]=%d,hs->offset[1]=%d",hs->offset[0],hs->offset[1]);
		ASSERT(FALSE);
	}

    return( ret );
}

#endif
#endif//#ifdef _SUPPORT_SSL
