/*
 *  Multi-precision integer library
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
 *  This MPI implementation is based on:
 *
 *  http://www.cacr.math.uwaterloo.ca/hac/about/chap14.pdf
 *  http://www.stillhq.com/extracted/gnupg-api/mpi_EX/
 *  http://math.libtomcrypt.com/files/tommath.pdf
 */

#include "stdafx.h"
#ifdef _SUPPORT_SSL
#include "config.h"

#if defined(POLARSSL_BIGNUM_C)

#include "bignum.h"
#include "bn_mul.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define ciL    ((int) sizeof(t_int))    /* chars in limb  */
#define biL    (ciL << 3)               /* bits  in limb  */
#define biH    (ciL << 2)               /* half limb size */

/*
 * Convert between bits/chars and number of limbs
 */
#define BITS_TO_LIMBS(i)  (((i) + biL - 1) / biL)
#define CHARS_TO_LIMBS(i) (((i) + ciL - 1) / ciL)

/*
 * Initialize one or more mpi_EX
 */
void mpi_init_EX( mpi_EX *X, ... )
{
    va_list args;

    va_start( args, X );

    while( X != NULL )
    {
        X->s = 1;
        X->n = 0;
        X->p = NULL;

        X = va_arg( args, mpi_EX* );
    }

    va_end( args );
}

/*
 * Unallocate one or more mpi_EX
 */
void mpi_free_EX( mpi_EX *X, ... )
{
    va_list args;

    va_start( args, X );

    while( X != NULL )
    {
        if( X->p != NULL )
        {
            memset( X->p, 0, X->n * ciL );
            free( X->p );
        }

        X->s = 1;
        X->n = 0;
        X->p = NULL;

        X = va_arg( args, mpi_EX* );
    }

    va_end( args );
}

/*
 * Enlarge to the specified number of limbs
 */
int mpi_grow_EX( mpi_EX *X, int nblimbs )
{
    t_int *p;

    if( X->n < nblimbs )
    {
        if( ( p = (t_int *) malloc( nblimbs * ciL ) ) == NULL )
            return( 1 );

        memset( p, 0, nblimbs * ciL );

        if( X->p != NULL )
        {
            memcpy( p, X->p, X->n * ciL );
            memset( X->p, 0, X->n * ciL );
            free( X->p );
        }

        X->n = nblimbs;
        X->p = p;
    }

    return( 0 );
}

/*
 * Copy the contents of Y into X
 */
int mpi_copy_EX( mpi_EX *X, mpi_EX *Y )
{
    int ret, i;

    if( X == Y )
        return( 0 );

    for( i = Y->n - 1; i > 0; i-- )
        if( Y->p[i] != 0 )
            break;
    i++;

    X->s = Y->s;

    MPI_CHK( mpi_grow_EX( X, i ) );

    memset( X->p, 0, X->n * ciL );
    memcpy( X->p, Y->p, i * ciL );

cleanup:

    return( ret );
}

/*
 * Swap the contents of X and Y
 */
void mpi_swap_EX( mpi_EX *X, mpi_EX *Y )
{
    mpi_EX T;

    memcpy( &T,  X, sizeof( mpi_EX ) );
    memcpy(  X,  Y, sizeof( mpi_EX ) );
    memcpy(  Y, &T, sizeof( mpi_EX ) );
}

/*
 * Set value from integer
 */
int mpi_lset_EX( mpi_EX *X, int z )
{
    int ret;

    MPI_CHK( mpi_grow_EX( X, 1 ) );
    memset( X->p, 0, X->n * ciL );

    X->p[0] = ( z < 0 ) ? -z : z;
    X->s    = ( z < 0 ) ? -1 : 1;

cleanup:

    return( ret );
}

/*
 * Return the number of least significant bits
 */
int mpi_lsb_EX( mpi_EX *X )
{
    int i, j, count = 0;

    for( i = 0; i < X->n; i++ )
        for( j = 0; j < (int) biL; j++, count++ )
            if( ( ( X->p[i] >> j ) & 1 ) != 0 )
                return( count );

    return( 0 );
}

/*
 * Return the number of most significant bits
 */
int mpi_msb_EX( mpi_EX *X )
{
    int i, j;

    for( i = X->n - 1; i > 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = biL - 1; j >= 0; j-- )
        if( ( ( X->p[i] >> j ) & 1 ) != 0 )
            break;

    return( ( i * biL ) + j + 1 );
}

/*
 * Return the total size in bytes
 */
int mpi_size_EX( mpi_EX *X )
{
    return( ( mpi_msb_EX( X ) + 7 ) >> 3 );
}

#if 0

/*
 * Convert an ASCII character to digit value
 */
static int mpi_get_digit( t_int *d, int radix, char c )
{
    *d = 255;

    if( c >= 0x30 && c <= 0x39 ) *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 ) *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 ) *d = c - 0x57;

    if( *d >= (t_int) radix )
        return( POLARSSL_ERR_MPI_INVALID_CHARACTER );

    return( 0 );
}

/*
 * Import from an ASCII string
 */
int mpi_read_string_EX( mpi_EX *X, int radix, char *s )
{
    int ret, i, j, n;
    t_int d;
    mpi_EX T;

    if( radix < 2 || radix > 16 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    mpi_init_EX( &T, NULL );

    if( radix == 16 )
    {
        n = BITS_TO_LIMBS( strlen( s ) << 2 );

        MPI_CHK( mpi_grow_EX( X, n ) );
        MPI_CHK( mpi_lset_EX( X, 0 ) );

        for( i = strlen( s ) - 1, j = 0; i >= 0; i--, j++ )
        {
            if( i == 0 && s[i] == '-' )
            {
                X->s = -1;
                break;
            }

            MPI_CHK( mpi_get_digit( &d, radix, s[i] ) );
            X->p[j / (2 * ciL)] |= d << ( (j % (2 * ciL)) << 2 );
        }
    }
    else
    {
        MPI_CHK( mpi_lset_EX( X, 0 ) );

        for( i = 0; i < (int) strlen( s ); i++ )
        {
            if( i == 0 && s[i] == '-' )
            {
                X->s = -1;
                continue;
            }

            MPI_CHK( mpi_get_digit( &d, radix, s[i] ) );
            MPI_CHK( mpi_mul_int_EX( &T, X, radix ) );

            if( X->s == 1 )
            {
                MPI_CHK( mpi_add_int_EX( X, &T, d ) );
            }
            else
            {
                MPI_CHK( mpi_sub_int_EX( X, &T, d ) );
            }
        }
    }

cleanup:

    mpi_free_EX( &T, NULL );

    return( ret );
}

/*
 * Helper to write the digits high-order first
 */
static int mpi_write_hlp( mpi_EX *X, int radix, char **p )
{
    int ret;
    t_int r;

    if( radix < 2 || radix > 16 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    MPI_CHK( mpi_mod_int_EX( &r, X, radix ) );
    MPI_CHK( mpi_div_int_EX( X, NULL, X, radix ) );

    if( mpi_cmp_int_EX( X, 0 ) != 0 )
        MPI_CHK( mpi_write_hlp( X, radix, p ) );

    if( r < 10 )
        *(*p)++ = (char)( r + 0x30 );
    else
        *(*p)++ = (char)( r + 0x37 );

cleanup:

    return( ret );
}

/*
 * Export into an ASCII string
 */
int mpi_write_string_EX( mpi_EX *X, int radix, char *s, int *slen )
{
    int ret = 0, n;
    char *p;
    mpi_EX T;

    if( radix < 2 || radix > 16 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    n = mpi_msb_EX( X );
    if( radix >=  4 ) n >>= 1;
    if( radix >= 16 ) n >>= 1;
    n += 3;

    if( *slen < n )
    {
        *slen = n;
        return( POLARSSL_ERR_MPI_BUFFER_TOO_SMALL );
    }

    p = s;
    mpi_init_EX( &T, NULL );

    if( X->s == -1 )
        *p++ = '-';

    if( radix == 16 )
    {
        int c, i, j, k;

        for( i = X->n - 1, k = 0; i >= 0; i-- )
        {
            for( j = ciL - 1; j >= 0; j-- )
            {
                c = ( X->p[i] >> (j << 3) ) & 0xFF;

                if( c == 0 && k == 0 && (i + j) != 0 )
                    continue;

                p += sprintf( p, "%02X", c );
                k = 1;
            }
        }
    }
    else
    {
        MPI_CHK( mpi_copy_EX( &T, X ) );

        if( T.s == -1 )
            T.s = 1;

        MPI_CHK( mpi_write_hlp( &T, radix, &p ) );
    }

    *p++ = '\0';
    *slen = p - s;

cleanup:

    mpi_free_EX( &T, NULL );

    return( ret );
}

/*
 * Read X from an opened file
 */
int mpi_read_file_EX( mpi_EX *X, int radix, FILE *fin )
{
    t_int d;
    int slen;
    char *p;
    char s[1024];

    memset( s, 0, sizeof( s ) );
    if( fgets( s, sizeof( s ) - 1, fin ) == NULL )
        return( POLARSSL_ERR_MPI_FILE_IO_ERROR );

    slen = strlen( s );
    if( s[slen - 1] == '\n' ) { slen--; s[slen] = '\0'; }
    if( s[slen - 1] == '\r' ) { slen--; s[slen] = '\0'; }

    p = s + slen;
    while( --p >= s )
        if( mpi_get_digit( &d, radix, *p ) != 0 )
            break;

    return( mpi_read_string_EX( X, radix, p + 1 ) );
}

/*
 * Write X into an opened file (or stdout if fout == NULL)
 */
int mpi_write_file( char *p, mpi_EX *X, int radix, FILE *fout )
{
    int n, ret;
    size_t slen;
    size_t plen;
    char s[1024];

    n = sizeof( s );
    memset( s, 0, n );
    n -= 2;

    MPI_CHK( mpi_write_string_EX( X, radix, s, (int *) &n ) );

    if( p == NULL ) p = "";

    plen = strlen( p );
    slen = strlen( s );
    s[slen++] = '\r';
    s[slen++] = '\n';

    if( fout != NULL )
    {
        if( fwrite( p, 1, plen, fout ) != plen ||
            fwrite( s, 1, slen, fout ) != slen )
            return( POLARSSL_ERR_MPI_FILE_IO_ERROR );
    }
    else
        printf( "%s%s", p, s );

cleanup:

    return( ret );
}
#endif 
/*
 * Import X from unsigned binary data, big endian
 */
int mpi_read_binary_EX( mpi_EX *X, unsigned char *buf, int buflen )
{
    int ret, i, j, n;

    for( n = 0; n < buflen; n++ )
        if( buf[n] != 0 )
            break;

    MPI_CHK( mpi_grow_EX( X, CHARS_TO_LIMBS( buflen - n ) ) );
    MPI_CHK( mpi_lset_EX( X, 0 ) );

    for( i = buflen - 1, j = 0; i >= n; i--, j++ )
        X->p[j / ciL] |= ((t_int) buf[i]) << ((j % ciL) << 3);

cleanup:

    return( ret );
}

/*
 * Export X into unsigned binary data, big endian
 */
int mpi_write_binary_EX( mpi_EX *X, unsigned char *buf, int buflen )
{
    int i, j, n;

    n = mpi_size_EX( X );

    if( buflen < n )
        return( POLARSSL_ERR_MPI_BUFFER_TOO_SMALL );

    memset( buf, 0, buflen );

    for( i = buflen - 1, j = 0; n > 0; i--, j++, n-- )
        buf[i] = (unsigned char)( X->p[j / ciL] >> ((j % ciL) << 3) );

    return( 0 );
}

/*
 * Left-shift: X <<= count
 */
int mpi_shift_l_EX( mpi_EX *X, int count )
{
    int ret, i, v0, t1;
    t_int r0 = 0, r1;

    v0 = count / (biL    );
    t1 = count & (biL - 1);

    i = mpi_msb_EX( X ) + count;

    if( X->n * (int) biL < i )
        MPI_CHK( mpi_grow_EX( X, BITS_TO_LIMBS( i ) ) );

    ret = 0;

    /*
     * shift by count / limb_size
     */
    if( v0 > 0 )
    {
        for( i = X->n - 1; i >= v0; i-- )
            X->p[i] = X->p[i - v0];

        for( ; i >= 0; i-- )
            X->p[i] = 0;
    }

    /*
     * shift by count % limb_size
     */
    if( t1 > 0 )
    {
        for( i = v0; i < X->n; i++ )
        {
            r1 = X->p[i] >> (biL - t1);
            X->p[i] <<= t1;
            X->p[i] |= r0;
            r0 = r1;
        }
    }

cleanup:

    return( ret );
}

/*
 * Right-shift: X >>= count
 */
int mpi_shift_r_EX( mpi_EX *X, int count )
{
    int i, v0, v1;
    t_int r0 = 0, r1;

    v0 = count /  biL;
    v1 = count & (biL - 1);

    /*
     * shift by count / limb_size
     */
    if( v0 > 0 )
    {
        for( i = 0; i < X->n - v0; i++ )
            X->p[i] = X->p[i + v0];

        for( ; i < X->n; i++ )
            X->p[i] = 0;
    }

    /*
     * shift by count % limb_size
     */
    if( v1 > 0 )
    {
        for( i = X->n - 1; i >= 0; i-- )
        {
            r1 = X->p[i] << (biL - v1);
            X->p[i] >>= v1;
            X->p[i] |= r0;
            r0 = r1;
        }
    }

    return( 0 );
}

/*
 * Compare unsigned values
 */
int mpi_cmp_abs_EX( mpi_EX *X, mpi_EX *Y )
{
    int i, j;

    for( i = X->n - 1; i >= 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = Y->n - 1; j >= 0; j-- )
        if( Y->p[j] != 0 )
            break;

    if( i < 0 && j < 0 )
        return( 0 );

    if( i > j ) return(  1 );
    if( j > i ) return( -1 );

    for( ; i >= 0; i-- )
    {
        if( X->p[i] > Y->p[i] ) return(  1 );
        if( X->p[i] < Y->p[i] ) return( -1 );
    }

    return( 0 );
}

/*
 * Compare signed values
 */
int mpi_cmp_mpi_EX( mpi_EX *X, mpi_EX *Y )
{
    int i, j;

    for( i = X->n - 1; i >= 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = Y->n - 1; j >= 0; j-- )
        if( Y->p[j] != 0 )
            break;

    if( i < 0 && j < 0 )
        return( 0 );

    if( i > j ) return(  X->s );
    if( j > i ) return( -X->s );

    if( X->s > 0 && Y->s < 0 ) return(  1 );
    if( Y->s > 0 && X->s < 0 ) return( -1 );

    for( ; i >= 0; i-- )
    {
        if( X->p[i] > Y->p[i] ) return(  X->s );
        if( X->p[i] < Y->p[i] ) return( -X->s );
    }

    return( 0 );
}

/*
 * Compare signed values
 */
int mpi_cmp_int_EX( mpi_EX *X, int z )
{
    mpi_EX Y;
    t_int p[1];

    *p  = ( z < 0 ) ? -z : z;
    Y.s = ( z < 0 ) ? -1 : 1;
    Y.n = 1;
    Y.p = p;

    return( mpi_cmp_mpi_EX( X, &Y ) );
}

/*
 * Unsigned addition: X = |A| + |B|  (HAC 14.7)
 */
int mpi_add_abs_EX( mpi_EX *X, mpi_EX *A, mpi_EX *B )
{
    int ret, i, j;
    t_int *o, *p, c;

    if( X == B )
    {
        mpi_EX *T = A; A = X; B = T;
    }

    if( X != A )
        MPI_CHK( mpi_copy_EX( X, A ) );
   
    /*
     * X should always be positive as a result of unsigned additions.
     */
    X->s = 1;

    for( j = B->n - 1; j >= 0; j-- )
        if( B->p[j] != 0 )
            break;

    MPI_CHK( mpi_grow_EX( X, j + 1 ) );

    o = B->p; p = X->p; c = 0;

    for( i = 0; i <= j; i++, o++, p++ )
    {
        *p +=  c; c  = ( *p <  c );
        *p += *o; c += ( *p < *o );
    }

    while( c != 0 )
    {
        if( i >= X->n )
        {
            MPI_CHK( mpi_grow_EX( X, i + 1 ) );
            p = X->p + i;
        }

        *p += c; c = ( *p < c ); i++;
    }

cleanup:

    return( ret );
}

/*
 * Helper for mpi_EX substraction
 */
static void mpi_sub_hlp( int n, t_int *s, t_int *d )
{
    int i;
    t_int c, z;

    for( i = c = 0; i < n; i++, s++, d++ )
    {
        z = ( *d <  c );     *d -=  c;
        c = ( *d < *s ) + z; *d -= *s;
    }

    while( c != 0 )
    {
        z = ( *d < c ); *d -= c;
        c = z; i++; d++;
    }
}

/*
 * Unsigned substraction: X = |A| - |B|  (HAC 14.9)
 */
int mpi_sub_abs_EX( mpi_EX *X, mpi_EX *A, mpi_EX *B )
{
    mpi_EX TB;
    int ret, n;

    if( mpi_cmp_abs_EX( A, B ) < 0 )
        return( POLARSSL_ERR_MPI_NEGATIVE_VALUE );

    mpi_init_EX( &TB, NULL );

    if( X == B )
    {
        MPI_CHK( mpi_copy_EX( &TB, B ) );
        B = &TB;
    }

    if( X != A )
        MPI_CHK( mpi_copy_EX( X, A ) );

    /*
     * X should always be positive as a result of unsigned substractions.
     */
    X->s = 1;

    ret = 0;

    for( n = B->n - 1; n >= 0; n-- )
        if( B->p[n] != 0 )
            break;

    mpi_sub_hlp( n + 1, B->p, X->p );

cleanup:

    mpi_free_EX( &TB, NULL );

    return( ret );
}

/*
 * Signed addition: X = A + B
 */
int mpi_add_mpi_EX( mpi_EX *X, mpi_EX *A, mpi_EX *B )
{
    int ret, s = A->s;

    if( A->s * B->s < 0 )
    {
        if( mpi_cmp_abs_EX( A, B ) >= 0 )
        {
            MPI_CHK( mpi_sub_abs_EX( X, A, B ) );
            X->s =  s;
        }
        else
        {
            MPI_CHK( mpi_sub_abs_EX( X, B, A ) );
            X->s = -s;
        }
    }
    else
    {
        MPI_CHK( mpi_add_abs_EX( X, A, B ) );
        X->s = s;
    }

cleanup:

    return( ret );
}

/*
 * Signed substraction: X = A - B
 */
int mpi_sub_mpi_EX( mpi_EX *X, mpi_EX *A, mpi_EX *B )
{
    int ret, s = A->s;

    if( A->s * B->s > 0 )
    {
        if( mpi_cmp_abs_EX( A, B ) >= 0 )
        {
            MPI_CHK( mpi_sub_abs_EX( X, A, B ) );
            X->s =  s;
        }
        else
        {
            MPI_CHK( mpi_sub_abs_EX( X, B, A ) );
            X->s = -s;
        }
    }
    else
    {
        MPI_CHK( mpi_add_abs_EX( X, A, B ) );
        X->s = s;
    }

cleanup:

    return( ret );
}

/*
 * Signed addition: X = A + b
 */
int mpi_add_int_EX( mpi_EX *X, mpi_EX *A, int b )
{
    mpi_EX _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_add_mpi_EX( X, A, &_B ) );
}

/*
 * Signed substraction: X = A - b
 */
int mpi_sub_int_EX( mpi_EX *X, mpi_EX *A, int b )
{
    mpi_EX _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_sub_mpi_EX( X, A, &_B ) );
}

/*
 * Helper for mpi_EX multiplication
 */ 
static void mpi_mul_hlp( int i, t_int *s, t_int *d, t_int b )
{
    t_int c = 0, t = 0;

#if defined(MULADDC_HUIT)
    for( ; i >= 8; i -= 8 )
    {
        MULADDC_INIT
        MULADDC_HUIT
        MULADDC_STOP
    }

    for( ; i > 0; i-- )
    {
        MULADDC_INIT
        MULADDC_CORE
        MULADDC_STOP
    }
#else
    for( ; i >= 16; i -= 16 )
    {
        MULADDC_INIT
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE

        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_STOP
    }

    for( ; i >= 8; i -= 8 )
    {
        MULADDC_INIT
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE

        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_STOP
    }

    for( ; i > 0; i-- )
    {
        MULADDC_INIT
        MULADDC_CORE
        MULADDC_STOP
    }
#endif

    t++;

    do {
        *d += c; c = ( *d < c ); d++;
    }
    while( c != 0 );
}

/*
 * Baseline multiplication: X = A * B  (HAC 14.12)
 */
int mpi_mul_mpi_EX( mpi_EX *X, mpi_EX *A, mpi_EX *B )
{
    int ret, i, j;
    mpi_EX TA, TB;

    mpi_init_EX( &TA, &TB, NULL );

    if( X == A ) { MPI_CHK( mpi_copy_EX( &TA, A ) ); A = &TA; }
    if( X == B ) { MPI_CHK( mpi_copy_EX( &TB, B ) ); B = &TB; }

    for( i = A->n - 1; i >= 0; i-- )
        if( A->p[i] != 0 )
            break;

    for( j = B->n - 1; j >= 0; j-- )
        if( B->p[j] != 0 )
            break;

    MPI_CHK( mpi_grow_EX( X, i + j + 2 ) );
    MPI_CHK( mpi_lset_EX( X, 0 ) );

    for( i++; j >= 0; j-- )
        mpi_mul_hlp( i, A->p, X->p + j, B->p[j] );

    X->s = A->s * B->s;

cleanup:

    mpi_free_EX( &TB, &TA, NULL );

    return( ret );
}

/*
 * Baseline multiplication: X = A * b
 */
int mpi_mul_int_EX( mpi_EX *X, mpi_EX *A, t_int b )
{
    mpi_EX _B;
    t_int p[1];

    _B.s = 1;
    _B.n = 1;
    _B.p = p;
    p[0] = b;

    return( mpi_mul_mpi_EX( X, A, &_B ) );
}

/*
 * Division by mpi_EX: A = Q * B + R  (HAC 14.20)
 */
int mpi_div_mpi_EX( mpi_EX *Q, mpi_EX *R, mpi_EX *A, mpi_EX *B )
{
    int ret, i, n, t, k;
    mpi_EX X, Y, Z, T1, T2;

    if( mpi_cmp_int_EX( B, 0 ) == 0 )
        return( POLARSSL_ERR_MPI_DIVISION_BY_ZERO );

    mpi_init_EX( &X, &Y, &Z, &T1, &T2, NULL );

    if( mpi_cmp_abs_EX( A, B ) < 0 )
    {
        if( Q != NULL ) MPI_CHK( mpi_lset_EX( Q, 0 ) );
        if( R != NULL ) MPI_CHK( mpi_copy_EX( R, A ) );
        return( 0 );
    }

    MPI_CHK( mpi_copy_EX( &X, A ) );
    MPI_CHK( mpi_copy_EX( &Y, B ) );
    X.s = Y.s = 1;

    MPI_CHK( mpi_grow_EX( &Z, A->n + 2 ) );
    MPI_CHK( mpi_lset_EX( &Z,  0 ) );
    MPI_CHK( mpi_grow_EX( &T1, 2 ) );
    MPI_CHK( mpi_grow_EX( &T2, 3 ) );

    k = mpi_msb_EX( &Y ) % biL;
    if( k < (int) biL - 1 )
    {
        k = biL - 1 - k;
        MPI_CHK( mpi_shift_l_EX( &X, k ) );
        MPI_CHK( mpi_shift_l_EX( &Y, k ) );
    }
    else k = 0;

    n = X.n - 1;
    t = Y.n - 1;
    mpi_shift_l_EX( &Y, biL * (n - t) );

    while( mpi_cmp_mpi_EX( &X, &Y ) >= 0 )
    {
        Z.p[n - t]++;
        mpi_sub_mpi_EX( &X, &X, &Y );
    }
    mpi_shift_r_EX( &Y, biL * (n - t) );

    for( i = n; i > t ; i-- )
    {
        if( X.p[i] >= Y.p[t] )
            Z.p[i - t - 1] = ~0;
        else
        {
#if defined(POLARSSL_HAVE_LONGLONG)
            t_dbl r;

            r  = (t_dbl) X.p[i] << biL;
            r |= (t_dbl) X.p[i - 1];
            r /= Y.p[t];
            if( r > ((t_dbl) 1 << biL) - 1)
                r = ((t_dbl) 1 << biL) - 1;

            Z.p[i - t - 1] = (t_int) r;
#else
            /*
             * __udiv_qrnnd_c, from gmp/longlong.h
             */
            t_int q0, q1, r0, r1;
            t_int d0, d1, d, m;

            d  = Y.p[t];
            d0 = ( d << biH ) >> biH;
            d1 = ( d >> biH );

            q1 = X.p[i] / d1;
            r1 = X.p[i] - d1 * q1;
            r1 <<= biH;
            r1 |= ( X.p[i - 1] >> biH );

            m = q1 * d0;
            if( r1 < m )
            {
                q1--, r1 += d;
                while( r1 >= d && r1 < m )
                    q1--, r1 += d;
            }
            r1 -= m;

            q0 = r1 / d1;
            r0 = r1 - d1 * q0;
            r0 <<= biH;
            r0 |= ( X.p[i - 1] << biH ) >> biH;

            m = q0 * d0;
            if( r0 < m )
            {
                q0--, r0 += d;
                while( r0 >= d && r0 < m )
                    q0--, r0 += d;
            }
            r0 -= m;

            Z.p[i - t - 1] = ( q1 << biH ) | q0;
#endif
        }

        Z.p[i - t - 1]++;
        do
        {
            Z.p[i - t - 1]--;

            MPI_CHK( mpi_lset_EX( &T1, 0 ) );
            T1.p[0] = (t < 1) ? 0 : Y.p[t - 1];
            T1.p[1] = Y.p[t];
            MPI_CHK( mpi_mul_int_EX( &T1, &T1, Z.p[i - t - 1] ) );

            MPI_CHK( mpi_lset_EX( &T2, 0 ) );
            T2.p[0] = (i < 2) ? 0 : X.p[i - 2];
            T2.p[1] = (i < 1) ? 0 : X.p[i - 1];
            T2.p[2] = X.p[i];
        }
        while( mpi_cmp_mpi_EX( &T1, &T2 ) > 0 );

        MPI_CHK( mpi_mul_int_EX( &T1, &Y, Z.p[i - t - 1] ) );
        MPI_CHK( mpi_shift_l_EX( &T1,  biL * (i - t - 1) ) );
        MPI_CHK( mpi_sub_mpi_EX( &X, &X, &T1 ) );

        if( mpi_cmp_int_EX( &X, 0 ) < 0 )
        {
            MPI_CHK( mpi_copy_EX( &T1, &Y ) );
            MPI_CHK( mpi_shift_l_EX( &T1, biL * (i - t - 1) ) );
            MPI_CHK( mpi_add_mpi_EX( &X, &X, &T1 ) );
            Z.p[i - t - 1]--;
        }
    }

    if( Q != NULL )
    {
        mpi_copy_EX( Q, &Z );
        Q->s = A->s * B->s;
    }

    if( R != NULL )
    {
        mpi_shift_r_EX( &X, k );
        mpi_copy_EX( R, &X );

        R->s = A->s;
        if( mpi_cmp_int_EX( R, 0 ) == 0 )
            R->s = 1;
    }

cleanup:

    mpi_free_EX( &X, &Y, &Z, &T1, &T2, NULL );

    return( ret );
}

/*
 * Division by int: A = Q * b + R
 *
 * Returns 0 if successful
 *         1 if memory allocation failed
 *         POLARSSL_ERR_MPI_DIVISION_BY_ZERO if b == 0
 */
int mpi_div_int_EX( mpi_EX *Q, mpi_EX *R, mpi_EX *A, int b )
{
    mpi_EX _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_div_mpi_EX( Q, R, A, &_B ) );
}

/*
 * Modulo: R = A mod B
 */
int mpi_mod_mpi_EX( mpi_EX *R, mpi_EX *A, mpi_EX *B )
{
    int ret;

    if( mpi_cmp_int_EX( B, 0 ) < 0 )
        return POLARSSL_ERR_MPI_NEGATIVE_VALUE;

    MPI_CHK( mpi_div_mpi_EX( NULL, R, A, B ) );

    while( mpi_cmp_int_EX( R, 0 ) < 0 )
      MPI_CHK( mpi_add_mpi_EX( R, R, B ) );

    while( mpi_cmp_mpi_EX( R, B ) >= 0 )
      MPI_CHK( mpi_sub_mpi_EX( R, R, B ) );

cleanup:

    return( ret );
}

/*
 * Modulo: r = A mod b
 */
int mpi_mod_int_EX( t_int *r, mpi_EX *A, int b )
{
    int i;
    t_int x, y, z;

    if( b == 0 )
        return( POLARSSL_ERR_MPI_DIVISION_BY_ZERO );

    if( b < 0 )
        return POLARSSL_ERR_MPI_NEGATIVE_VALUE;

    /*
     * handle trivial cases
     */
    if( b == 1 )
    {
        *r = 0;
        return( 0 );
    }

    if( b == 2 )
    {
        *r = A->p[0] & 1;
        return( 0 );
    }

    /*
     * general case
     */
    for( i = A->n - 1, y = 0; i >= 0; i-- )
    {
        x  = A->p[i];
        y  = ( y << biH ) | ( x >> biH );
        z  = y / b;
        y -= z * b;

        x <<= biH;
        y  = ( y << biH ) | ( x >> biH );
        z  = y / b;
        y -= z * b;
    }

    /*
     * If A is negative, then the current y represents a negative value.
     * Flipping it to the positive side.
     */
    if( A->s < 0 && y != 0 )
        y = b - y;

    *r = y;

    return( 0 );
}

/*
 * Fast Montgomery initialization (thanks to Tom St Denis)
 */
static void mpi_montg_init( t_int *mm, mpi_EX *N )
{
    t_int x, m0 = N->p[0];

    x  = m0;
    x += ( ( m0 + 2 ) & 4 ) << 1;
    x *= ( 2 - ( m0 * x ) );
#ifdef _MSC_VER
#pragma warning(disable:6326)
	//warning C6326: Potential comparison of a constant with another constant
#endif
    if( biL >= 16 ) x *= ( 2 - ( m0 * x ) );
    if( biL >= 32 ) x *= ( 2 - ( m0 * x ) );
    if( biL >= 64 ) x *= ( 2 - ( m0 * x ) );
#ifdef _MSC_VER
#pragma warning(default:6326)
#endif

    *mm = ~x + 1;
}

/*
 * Montgomery multiplication: A = A * B * R^-1 mod N  (HAC 14.36)
 */
static void mpi_montmul( mpi_EX *A, mpi_EX *B, mpi_EX *N, t_int mm, mpi_EX *T )
{
    int i, n, m;
    t_int u0, u1, *d;

    memset( T->p, 0, T->n * ciL );

    d = T->p;
    n = N->n;
    m = ( B->n < n ) ? B->n : n;

    for( i = 0; i < n; i++ )
    {
        /*
         * T = (T + u0*B + u1*N) / 2^biL
         */
        u0 = A->p[i];
        u1 = ( d[0] + u0 * B->p[0] ) * mm;

        mpi_mul_hlp( m, B->p, d, u0 );
        mpi_mul_hlp( n, N->p, d, u1 );

        *d++ = u0; d[n + 1] = 0;
    }

    memcpy( A->p, d, (n + 1) * ciL );

    if( mpi_cmp_abs_EX( A, N ) >= 0 )
        mpi_sub_hlp( n, N->p, A->p );
    else
        /* prevent timing attacks */
        mpi_sub_hlp( n, A->p, T->p );
}

/*
 * Montgomery reduction: A = A * R^-1 mod N
 */
static void mpi_montred( mpi_EX *A, mpi_EX *N, t_int mm, mpi_EX *T )
{
    t_int z = 1;
    mpi_EX U;

    U.n = U.s = z;
    U.p = &z;

    mpi_montmul( A, &U, N, mm, T );
}

/*
 * Sliding-window exponentiation: X = A^E mod N  (HAC 14.85)
 */
int mpi_exp_mod_EX( mpi_EX *X, mpi_EX *A, mpi_EX *E, mpi_EX *N, mpi_EX *_RR )
{
    int ret, i, j, wsize, wbits;
    int bufsize, nblimbs, nbits;
    t_int ei, mm, state;
    mpi_EX RR, T, W[64];

    if( mpi_cmp_int_EX( N, 0 ) < 0 || ( N->p[0] & 1 ) == 0 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    /*
     * Init temps and window size
     */
    mpi_montg_init( &mm, N );
    mpi_init_EX( &RR, &T, NULL );
    memset( W, 0, sizeof( W ) );

    i = mpi_msb_EX( E );

    wsize = ( i > 671 ) ? 6 : ( i > 239 ) ? 5 :
            ( i >  79 ) ? 4 : ( i >  23 ) ? 3 : 1;

    j = N->n + 1;
    MPI_CHK( mpi_grow_EX( X, j ) );
    MPI_CHK( mpi_grow_EX( &W[1],  j ) );
    MPI_CHK( mpi_grow_EX( &T, j * 2 ) );

    /*
     * If 1st call, pre-compute R^2 mod N
     */
    if( _RR == NULL || _RR->p == NULL )
    {
        MPI_CHK( mpi_lset_EX( &RR, 1 ) );
        MPI_CHK( mpi_shift_l_EX( &RR, N->n * 2 * biL ) );
        MPI_CHK( mpi_mod_mpi_EX( &RR, &RR, N ) );

        if( _RR != NULL )
            memcpy( _RR, &RR, sizeof( mpi_EX ) );
    }
    else
        memcpy( &RR, _RR, sizeof( mpi_EX ) );

    /*
     * W[1] = A * R^2 * R^-1 mod N = A * R mod N
     */
    if( mpi_cmp_mpi_EX( A, N ) >= 0 )
        mpi_mod_mpi_EX( &W[1], A, N );
    else   mpi_copy_EX( &W[1], A );

    mpi_montmul( &W[1], &RR, N, mm, &T );

    /*
     * X = R^2 * R^-1 mod N = R mod N
     */
    MPI_CHK( mpi_copy_EX( X, &RR ) );
    mpi_montred( X, N, mm, &T );

    if( wsize > 1 )
    {
        /*
         * W[1 << (wsize - 1)] = W[1] ^ (wsize - 1)
         */
        j =  1 << (wsize - 1);

        MPI_CHK( mpi_grow_EX( &W[j], N->n + 1 ) );
        MPI_CHK( mpi_copy_EX( &W[j], &W[1]    ) );

        for( i = 0; i < wsize - 1; i++ )
            mpi_montmul( &W[j], &W[j], N, mm, &T );
    
        /*
         * W[i] = W[i - 1] * W[1]
         */
        for( i = j + 1; i < (1 << wsize); i++ )
        {
            MPI_CHK( mpi_grow_EX( &W[i], N->n + 1 ) );
            MPI_CHK( mpi_copy_EX( &W[i], &W[i - 1] ) );

            mpi_montmul( &W[i], &W[1], N, mm, &T );
        }
    }

    nblimbs = E->n;
    bufsize = 0;
    nbits   = 0;
    wbits   = 0;
    state   = 0;

    while( 1 )
    {
        if( bufsize == 0 )
        {
            if( nblimbs-- == 0 )
                break;

            bufsize = sizeof( t_int ) << 3;
        }

        bufsize--;

        ei = (E->p[nblimbs] >> bufsize) & 1;

        /*
         * skip leading 0s
         */
        if( ei == 0 && state == 0 )
            continue;

        if( ei == 0 && state == 1 )
        {
            /*
             * out of window, square X
             */
            mpi_montmul( X, X, N, mm, &T );
            continue;
        }

        /*
         * add ei to current window
         */
        state = 2;

        nbits++;
        wbits |= (ei << (wsize - nbits));

        if( nbits == wsize )
        {
            /*
             * X = X^wsize R^-1 mod N
             */
            for( i = 0; i < wsize; i++ )
                mpi_montmul( X, X, N, mm, &T );

            /*
             * X = X * W[wbits] R^-1 mod N
             */
            mpi_montmul( X, &W[wbits], N, mm, &T );

            state--;
            nbits = 0;
            wbits = 0;
        }
    }

    /*
     * process the remaining bits
     */
    for( i = 0; i < nbits; i++ )
    {
        mpi_montmul( X, X, N, mm, &T );

        wbits <<= 1;

        if( (wbits & (1 << wsize)) != 0 )
            mpi_montmul( X, &W[1], N, mm, &T );
    }

    /*
     * X = A^E * R * R^-1 mod N = A^E mod N
     */
    mpi_montred( X, N, mm, &T );

cleanup:

    for( i = (1 << (wsize - 1)); i < (1 << wsize); i++ )
        mpi_free_EX( &W[i], NULL );

    if( _RR != NULL )
         mpi_free_EX( &W[1], &T, NULL );
    else mpi_free_EX( &W[1], &T, &RR, NULL );

    return( ret );
}

/*
 * Greatest common divisor: G = gcd(A, B)  (HAC 14.54)
 */
int mpi_gcd_EX( mpi_EX *G, mpi_EX *A, mpi_EX *B )
{
    int ret, lz, lzt;
    mpi_EX TG, TA, TB;

    mpi_init_EX( &TG, &TA, &TB, NULL );

    MPI_CHK( mpi_copy_EX( &TA, A ) );
    MPI_CHK( mpi_copy_EX( &TB, B ) );

    lz = mpi_lsb_EX( &TA );
    lzt = mpi_lsb_EX( &TB );

    if ( lzt < lz )
        lz = lzt;

    MPI_CHK( mpi_shift_r_EX( &TA, lz ) );
    MPI_CHK( mpi_shift_r_EX( &TB, lz ) );

    TA.s = TB.s = 1;

    while( mpi_cmp_int_EX( &TA, 0 ) != 0 )
    {
        MPI_CHK( mpi_shift_r_EX( &TA, mpi_lsb_EX( &TA ) ) );
        MPI_CHK( mpi_shift_r_EX( &TB, mpi_lsb_EX( &TB ) ) );

        if( mpi_cmp_mpi_EX( &TA, &TB ) >= 0 )
        {
            MPI_CHK( mpi_sub_abs_EX( &TA, &TA, &TB ) );
            MPI_CHK( mpi_shift_r_EX( &TA, 1 ) );
        }
        else
        {
            MPI_CHK( mpi_sub_abs_EX( &TB, &TB, &TA ) );
            MPI_CHK( mpi_shift_r_EX( &TB, 1 ) );
        }
    }

    MPI_CHK( mpi_shift_l_EX( &TB, lz ) );
    MPI_CHK( mpi_copy_EX( G, &TB ) );

cleanup:

    mpi_free_EX( &TB, &TA, &TG, NULL );

    return( ret );
}

#if defined(POLARSSL_GENPRIME)

/*
 * Modular inverse: X = A^-1 mod N  (HAC 14.61 / 14.64)
 */
int mpi_inv_mod_EX( mpi_EX *X, mpi_EX *A, mpi_EX *N )
{
    int ret;
    mpi_EX G, TA, TU, U1, U2, TB, TV, V1, V2;

    if( mpi_cmp_int_EX( N, 0 ) <= 0 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    mpi_init_EX( &TA, &TU, &U1, &U2, &G,
              &TB, &TV, &V1, &V2, NULL );

    MPI_CHK( mpi_gcd_EX( &G, A, N ) );

    if( mpi_cmp_int_EX( &G, 1 ) != 0 )
    {
        ret = POLARSSL_ERR_MPI_NOT_ACCEPTABLE;
        goto cleanup;
    }

    MPI_CHK( mpi_mod_mpi_EX( &TA, A, N ) );
    MPI_CHK( mpi_copy_EX( &TU, &TA ) );
    MPI_CHK( mpi_copy_EX( &TB, N ) );
    MPI_CHK( mpi_copy_EX( &TV, N ) );

    MPI_CHK( mpi_lset_EX( &U1, 1 ) );
    MPI_CHK( mpi_lset_EX( &U2, 0 ) );
    MPI_CHK( mpi_lset_EX( &V1, 0 ) );
    MPI_CHK( mpi_lset_EX( &V2, 1 ) );

    do
    {
        while( ( TU.p[0] & 1 ) == 0 )
        {
            MPI_CHK( mpi_shift_r_EX( &TU, 1 ) );

            if( ( U1.p[0] & 1 ) != 0 || ( U2.p[0] & 1 ) != 0 )
            {
                MPI_CHK( mpi_add_mpi_EX( &U1, &U1, &TB ) );
                MPI_CHK( mpi_sub_mpi_EX( &U2, &U2, &TA ) );
            }

            MPI_CHK( mpi_shift_r_EX( &U1, 1 ) );
            MPI_CHK( mpi_shift_r_EX( &U2, 1 ) );
        }

        while( ( TV.p[0] & 1 ) == 0 )
        {
            MPI_CHK( mpi_shift_r_EX( &TV, 1 ) );

            if( ( V1.p[0] & 1 ) != 0 || ( V2.p[0] & 1 ) != 0 )
            {
                MPI_CHK( mpi_add_mpi_EX( &V1, &V1, &TB ) );
                MPI_CHK( mpi_sub_mpi_EX( &V2, &V2, &TA ) );
            }

            MPI_CHK( mpi_shift_r_EX( &V1, 1 ) );
            MPI_CHK( mpi_shift_r_EX( &V2, 1 ) );
        }

        if( mpi_cmp_mpi_EX( &TU, &TV ) >= 0 )
        {
            MPI_CHK( mpi_sub_mpi_EX( &TU, &TU, &TV ) );
            MPI_CHK( mpi_sub_mpi_EX( &U1, &U1, &V1 ) );
            MPI_CHK( mpi_sub_mpi_EX( &U2, &U2, &V2 ) );
        }
        else
        {
            MPI_CHK( mpi_sub_mpi_EX( &TV, &TV, &TU ) );
            MPI_CHK( mpi_sub_mpi_EX( &V1, &V1, &U1 ) );
            MPI_CHK( mpi_sub_mpi_EX( &V2, &V2, &U2 ) );
        }
    }
    while( mpi_cmp_int_EX( &TU, 0 ) != 0 );

    while( mpi_cmp_int_EX( &V1, 0 ) < 0 )
        MPI_CHK( mpi_add_mpi_EX( &V1, &V1, N ) );

    while( mpi_cmp_mpi_EX( &V1, N ) >= 0 )
        MPI_CHK( mpi_sub_mpi_EX( &V1, &V1, N ) );

    MPI_CHK( mpi_copy_EX( X, &V1 ) );

cleanup:

    mpi_free_EX( &V2, &V1, &TV, &TB, &G,
              &U2, &U1, &TU, &TA, NULL );

    return( ret );
}

static const int small_prime[] =
{
        3,    5,    7,   11,   13,   17,   19,   23,
       29,   31,   37,   41,   43,   47,   53,   59,
       61,   67,   71,   73,   79,   83,   89,   97,
      101,  103,  107,  109,  113,  127,  131,  137,
      139,  149,  151,  157,  163,  167,  173,  179,
      181,  191,  193,  197,  199,  211,  223,  227,
      229,  233,  239,  241,  251,  257,  263,  269,
      271,  277,  281,  283,  293,  307,  311,  313,
      317,  331,  337,  347,  349,  353,  359,  367,
      373,  379,  383,  389,  397,  401,  409,  419,
      421,  431,  433,  439,  443,  449,  457,  461,
      463,  467,  479,  487,  491,  499,  503,  509,
      521,  523,  541,  547,  557,  563,  569,  571,
      577,  587,  593,  599,  601,  607,  613,  617,
      619,  631,  641,  643,  647,  653,  659,  661,
      673,  677,  683,  691,  701,  709,  719,  727,
      733,  739,  743,  751,  757,  761,  769,  773,
      787,  797,  809,  811,  821,  823,  827,  829,
      839,  853,  857,  859,  863,  877,  881,  883,
      887,  907,  911,  919,  929,  937,  941,  947,
      953,  967,  971,  977,  983,  991,  997, -103
};

/*
 * Miller-Rabin primality test  (HAC 4.24)
 */
int mpi_is_prime_EX( mpi_EX *X, int (*f_rng)(void *), void *p_rng )
{
    int ret, i, j, n, s, xs;
    mpi_EX W, R, T, A, RR;
    unsigned char *p;

    if( mpi_cmp_int_EX( X, 0 ) == 0 ||
        mpi_cmp_int_EX( X, 1 ) == 0 )
        return( POLARSSL_ERR_MPI_NOT_ACCEPTABLE );

    if( mpi_cmp_int_EX( X, 2 ) == 0 )
        return( 0 );

    mpi_init_EX( &W, &R, &T, &A, &RR, NULL );

    xs = X->s; X->s = 1;

    /*
     * test trivial factors first
     */
    if( ( X->p[0] & 1 ) == 0 )
        return( POLARSSL_ERR_MPI_NOT_ACCEPTABLE );

    for( i = 0; small_prime[i] > 0; i++ )
    {
        t_int r;

        if( mpi_cmp_int_EX( X, small_prime[i] ) <= 0 )
            return( 0 );

        MPI_CHK( mpi_mod_int_EX( &r, X, small_prime[i] ) );

        if( r == 0 )
            return( POLARSSL_ERR_MPI_NOT_ACCEPTABLE );
    }

    /*
     * W = |X| - 1
     * R = W >> lsb( W )
     */
    s = mpi_lsb_EX( &W );
    MPI_CHK( mpi_sub_int_EX( &W, X, 1 ) );
    MPI_CHK( mpi_copy_EX( &R, &W ) );
    MPI_CHK( mpi_shift_r_EX( &R, s ) );

    i = mpi_msb_EX( X );
    /*
     * HAC, table 4.4
     */
    n = ( ( i >= 1300 ) ?  2 : ( i >=  850 ) ?  3 :
          ( i >=  650 ) ?  4 : ( i >=  350 ) ?  8 :
          ( i >=  250 ) ? 12 : ( i >=  150 ) ? 18 : 27 );

    for( i = 0; i < n; i++ )
    {
        /*
         * pick a random A, 1 < A < |X| - 1
         */
        MPI_CHK( mpi_grow_EX( &A, X->n ) );

        p = (unsigned char *) A.p;
        for( j = 0; j < A.n * ciL; j++ )
            *p++ = (unsigned char) f_rng( p_rng );

        j = mpi_msb_EX( &A ) - mpi_msb_EX( &W );
        MPI_CHK( mpi_shift_r_EX( &A, j + 1 ) );
        A.p[0] |= 3;

        /*
         * A = A^R mod |X|
         */
        MPI_CHK( mpi_exp_mod_EX( &A, &A, &R, X, &RR ) );

        if( mpi_cmp_mpi_EX( &A, &W ) == 0 ||
            mpi_cmp_int_EX( &A,  1 ) == 0 )
            continue;

        j = 1;
        while( j < s && mpi_cmp_mpi_EX( &A, &W ) != 0 )
        {
            /*
             * A = A * A mod |X|
             */
            MPI_CHK( mpi_mul_mpi_EX( &T, &A, &A ) );
            MPI_CHK( mpi_mod_mpi_EX( &A, &T, X  ) );

            if( mpi_cmp_int_EX( &A, 1 ) == 0 )
                break;

            j++;
        }

        /*
         * not prime if A != |X| - 1 or A == 1
         */
        if( mpi_cmp_mpi_EX( &A, &W ) != 0 ||
            mpi_cmp_int_EX( &A,  1 ) == 0 )
        {
            ret = POLARSSL_ERR_MPI_NOT_ACCEPTABLE;
            break;
        }
    }

cleanup:

    X->s = xs;

    mpi_free_EX( &RR, &A, &T, &R, &W, NULL );

    return( ret );
}

/*
 * Prime number generation
 */
int mpi_gen_prime_EX( mpi_EX *X, int nbits, int dh_flag,
                   int (*f_rng)(void *), void *p_rng )
{
    int ret, k, n;
    unsigned char *p;
    mpi_EX Y;

    if( nbits < 3 )
        return( POLARSSL_ERR_MPI_BAD_INPUT_DATA );

    mpi_init_EX( &Y, NULL );

    n = BITS_TO_LIMBS( nbits );

    MPI_CHK( mpi_grow_EX( X, n ) );
    MPI_CHK( mpi_lset_EX( X, 0 ) );

    p = (unsigned char *) X->p;
    for( k = 0; k < X->n * ciL; k++ )
        *p++ = (unsigned char) f_rng( p_rng );

    k = mpi_msb_EX( X );
    if( k < nbits ) MPI_CHK( mpi_shift_l_EX( X, nbits - k ) );
    if( k > nbits ) MPI_CHK( mpi_shift_r_EX( X, k - nbits ) );

    X->p[0] |= 3;

    if( dh_flag == 0 )
    {
        while( ( ret = mpi_is_prime_EX( X, f_rng, p_rng ) ) != 0 )
        {
            if( ret != POLARSSL_ERR_MPI_NOT_ACCEPTABLE )
                goto cleanup;

            MPI_CHK( mpi_add_int_EX( X, X, 2 ) );
        }
    }
    else
    {
        MPI_CHK( mpi_sub_int_EX( &Y, X, 1 ) );
        MPI_CHK( mpi_shift_r_EX( &Y, 1 ) );

        while( 1 )
        {
            if( ( ret = mpi_is_prime_EX( X, f_rng, p_rng ) ) == 0 )
            {
                if( ( ret = mpi_is_prime_EX( &Y, f_rng, p_rng ) ) == 0 )
                    break;

                if( ret != POLARSSL_ERR_MPI_NOT_ACCEPTABLE )
                    goto cleanup;
            }

            if( ret != POLARSSL_ERR_MPI_NOT_ACCEPTABLE )
                goto cleanup;

            MPI_CHK( mpi_add_int_EX( &Y, X, 1 ) );
            MPI_CHK( mpi_add_int_EX(  X, X, 2 ) );
            MPI_CHK( mpi_shift_r_EX( &Y, 1 ) );
        }
    }

cleanup:

    mpi_free_EX( &Y, NULL );

    return( ret );
}

#endif

#endif
#endif//#ifdef _SUPPORT_SSL
