#include "stdafx.h"
#ifdef _SUPPORT_SSL
/*
 *  SSLv3/TLSv1 client-side functions
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

#include "config.h"

#if defined(POLARSSL_SSL_CLI_C)

//#include "debug.h"
#include "ssl.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static int ssl_write_client_hello( ssl_context *ssl )
{
    int ret, i, n;
    unsigned char *buf;
    unsigned char *p;
    time_t t;

    SSL_DEBUG_MSG( 2, ( "=> write client hello" ) );

    ssl->major_ver = SSL_MAJOR_VERSION_3;
    ssl->minor_ver = SSL_MINOR_VERSION_0;

    ssl->max_major_ver = SSL_MAJOR_VERSION_3;
    ssl->max_minor_ver = SSL_MINOR_VERSION_1;

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   highest version supported
     *     6  .   9   current UNIX time
     *    10  .  37   random bytes
     */
    buf = ssl->out_msg;
    p = buf + 4;

    *p++ = (unsigned char) ssl->max_major_ver;
    *p++ = (unsigned char) ssl->max_minor_ver;

    SSL_DEBUG_MSG( 3, ( "client hello, max version: [%d:%d]",
                   buf[4], buf[5] ) );

    t = time( NULL );
    *p++ = (unsigned char)( t >> 24 );
    *p++ = (unsigned char)( t >> 16 );
    *p++ = (unsigned char)( t >>  8 );
    *p++ = (unsigned char)( t       );

    SSL_DEBUG_MSG( 3, ( "client hello, current time: %lu", t ) );

    for( i = 28; i > 0; i-- )
        *p++ = (unsigned char) ssl->f_rng( ssl->p_rng );

    memcpy( ssl->randbytes, buf + 6, 32 );

    SSL_DEBUG_BUF( 3, "client hello, random bytes", buf + 6, 32 );

    /*
     *    38  .  38   session id length
     *    39  . 39+n  session id
     *   40+n . 41+n  cipherlist length
     *   42+n . ..    cipherlist
     *   ..   . ..    compression alg. (0)
     *   ..   . ..    extensions (unused)
     */
    n = ssl->session->length;

    if( n < 16 || n > 32 || ssl->resume == 0 ||
        t - ssl->session->start > ssl->timeout )
        n = 0;

    *p++ = (unsigned char) n;

    for( i = 0; i < n; i++ )
        *p++ = ssl->session->id[i];

    SSL_DEBUG_MSG( 3, ( "client hello, session id len.: %d", n ) );
    SSL_DEBUG_BUF( 3,   "client hello, session id", buf + 39, n );

    for( n = 0; ssl->ciphers[n] != 0; n++ );
    *p++ = (unsigned char)( n >> 7 );
    *p++ = (unsigned char)( n << 1 );

    SSL_DEBUG_MSG( 3, ( "client hello, got %d ciphers", n ) );

    for( i = 0; i < n; i++ )
    {
        SSL_DEBUG_MSG( 3, ( "client hello, add cipher: %2d",
                       ssl->ciphers[i] ) );

        *p++ = (unsigned char)( ssl->ciphers[i] >> 8 );
        *p++ = (unsigned char)( ssl->ciphers[i]      );
    }

    SSL_DEBUG_MSG( 3, ( "client hello, compress len.: %d", 1 ) );
    SSL_DEBUG_MSG( 3, ( "client hello, compress alg.: %d", 0 ) );

    *p++ = 1;
    *p++ = SSL_COMPRESS_NULL;

    if ( ssl->hostname != NULL )
    {
        SSL_DEBUG_MSG( 3, ( "client hello, server name extension: %s",
                       ssl->hostname ) );

        *p++ = (unsigned char)( ( (ssl->hostname_len + 9) >> 8 ) & 0xFF );
        *p++ = (unsigned char)( ( (ssl->hostname_len + 9)      ) & 0xFF );

        *p++ = (unsigned char)( ( TLS_EXT_SERVERNAME >> 8 ) & 0xFF );
        *p++ = (unsigned char)( ( TLS_EXT_SERVERNAME      ) & 0xFF );

        *p++ = (unsigned char)( ( (ssl->hostname_len + 5) >> 8 ) & 0xFF );
        *p++ = (unsigned char)( ( (ssl->hostname_len + 5)      ) & 0xFF );

        *p++ = (unsigned char)( ( (ssl->hostname_len + 3) >> 8 ) & 0xFF );
        *p++ = (unsigned char)( ( (ssl->hostname_len + 3)      ) & 0xFF );

        *p++ = (unsigned char)( ( TLS_EXT_SERVERNAME_HOSTNAME ) & 0xFF );
        *p++ = (unsigned char)( ( ssl->hostname_len >> 8 ) & 0xFF );
        *p++ = (unsigned char)( ( ssl->hostname_len      ) & 0xFF );

        memcpy( p, ssl->hostname, ssl->hostname_len );

        p += ssl->hostname_len;
    }

    ssl->out_msglen  = p - buf;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CLIENT_HELLO;

    ssl->state++;

    if( ( ret = ssl_write_record( ssl ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "ssl_write_record", ret );
        return( ret );
    }

    SSL_DEBUG_MSG( 2, ( "<= write client hello" ) );

    return( 0 );
}

static int ssl_parse_server_hello( ssl_context *ssl )
{
    time_t t;
    int ret, i, n;
    int ext_len;
    unsigned char *buf;

    SSL_DEBUG_MSG( 2, ( "=> parse server hello" ) );

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   protocol version
     *     6  .   9   UNIX time()
     *    10  .  37   random bytes
     */
    buf = ssl->in_msg;

    if( ( ret = ssl_read_record( ssl ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "ssl_read_record", ret );
        return( ret );
    }

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
        return( POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE );
    }

    SSL_DEBUG_MSG( 3, ( "server hello, chosen version: [%d:%d]",
                   buf[4], buf[5] ) );

    if( ssl->in_hslen < 42 ||
        buf[0] != SSL_HS_SERVER_HELLO ||
        buf[4] != SSL_MAJOR_VERSION_3 )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
        return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO );
    }

    if( buf[5] != SSL_MINOR_VERSION_0 &&
        buf[5] != SSL_MINOR_VERSION_1 )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
        return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO );
    }

    ssl->minor_ver = buf[5];

    t = ( (time_t) buf[6] << 24 )
      | ( (time_t) buf[7] << 16 )
      | ( (time_t) buf[8] <<  8 )
      | ( (time_t) buf[9]       );

    memcpy( ssl->randbytes + 32, buf + 6, 32 );

    n = buf[38];

    SSL_DEBUG_MSG( 3, ( "server hello, current time: %lu", t ) );
    SSL_DEBUG_BUF( 3,   "server hello, random bytes", buf + 6, 32 );

    /*
     *    38  .  38   session id length
     *    39  . 38+n  session id
     *   39+n . 40+n  chosen cipher
     *   41+n . 41+n  chosen compression alg.
     *   42+n . 43+n  extensions length
     *   44+n . 44+n+m extensions
     */
    if( n < 0 || n > 32 || ssl->in_hslen > 42 + n )
    {
        ext_len = ( ( buf[42 + n] <<  8 )
                  | ( buf[43 + n]       ) ) + 2;
    }
    else
    {
        ext_len = 0;
    }

    if( n < 0 || n > 32 || ssl->in_hslen != 42 + n + ext_len )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
        return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO );
    }

    i = ( buf[39 + n] << 8 ) | buf[40 + n];

    SSL_DEBUG_MSG( 3, ( "server hello, session id len.: %d", n ) );
    SSL_DEBUG_BUF( 3,   "server hello, session id", buf + 39, n );

    /*
     * Check if the session can be resumed
     */
    if( ssl->resume == 0 || n == 0 ||
        ssl->session->cipher  != i ||
        ssl->session->length  != n ||
        memcmp( ssl->session->id, buf + 39, n ) != 0 )
    {
        ssl->state++;
        ssl->resume = 0;
        ssl->session->start = time( NULL );
        ssl->session->cipher = i;
        ssl->session->length = n;
        memcpy( ssl->session->id, buf + 39, n );
    }
    else
    {
        ssl->state = SSL_SERVER_CHANGE_CIPHER_SPEC;
        ssl_derive_keys( ssl );
    }

    SSL_DEBUG_MSG( 3, ( "%s session has been resumed",
                   ssl->resume ? "a" : "no" ) );

    SSL_DEBUG_MSG( 3, ( "server hello, chosen cipher: %d", i ) );
    SSL_DEBUG_MSG( 3, ( "server hello, compress alg.: %d", buf[41 + n] ) );

    i = 0;
    while( 1 )
    {
        if( ssl->ciphers[i] == 0 )
        {
            SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
            return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO );
        }

        if( ssl->ciphers[i++] == ssl->session->cipher )
            break;
    }

    if( buf[41 + n] != SSL_COMPRESS_NULL )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello message" ) );
        return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO );
    }

    /* TODO: Process extensions */

    SSL_DEBUG_MSG( 2, ( "<= parse server hello" ) );

    return( 0 );
}

static int ssl_parse_server_key_exchange( ssl_context *ssl )
{
    //int ret;
    //unsigned char *end;
    //unsigned char hash[36];
    //md5_context md5;
    //sha1_context sha1_EX;

    SSL_DEBUG_MSG( 2, ( "=> parse server key exchange" ) );

    if( ssl->session->cipher != SSL_EDH_RSA_DES_168_SHA &&
        ssl->session->cipher != SSL_EDH_RSA_AES_256_SHA &&
	ssl->session->cipher != SSL_EDH_RSA_CAMELLIA_256_SHA)
    {
        SSL_DEBUG_MSG( 2, ( "<= skip parse server key exchange" ) );
        ssl->state++;
        return( 0 );
    }

    SSL_DEBUG_MSG( 1, ( "support for dhm in not available" ) );
    return( POLARSSL_ERR_SSL_FEATURE_UNAVAILABLE );
}

static int ssl_parse_certificate_request( ssl_context *ssl )
{
    int ret;

    SSL_DEBUG_MSG( 2, ( "=> parse certificate request" ) );

    /*
     *     0  .   0   handshake type
     *     1  .   3   handshake length
     *     4  .   5   SSL version
     *     6  .   6   cert type count
     *     7  .. n-1  cert types
     *     n  .. n+1  length of all DNs
     *    n+2 .. n+3  length of DN 1
     *    n+4 .. ...  Distinguished Name #1
     *    ... .. ...  length of DN 2, etc.
     */
    if( ( ret = ssl_read_record( ssl ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "ssl_read_record", ret );
        return( ret );
    }

    if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
    {
        SSL_DEBUG_MSG( 1, ( "bad certificate request message" ) );
        return( POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE );
    }

    ssl->client_auth = 0;
    ssl->state++;

    if( ssl->in_msg[0] == SSL_HS_CERTIFICATE_REQUEST )
        ssl->client_auth++;

    SSL_DEBUG_MSG( 3, ( "got %s certificate request",
                        ssl->client_auth ? "a" : "no" ) );

    SSL_DEBUG_MSG( 2, ( "<= parse certificate request" ) );

    return( 0 );
}

static int ssl_parse_server_hello_done( ssl_context *ssl )
{
    int ret;

    SSL_DEBUG_MSG( 2, ( "=> parse server hello done" ) );

    if( ssl->client_auth != 0 )
    {
        if( ( ret = ssl_read_record( ssl ) ) != 0 )
        {
            SSL_DEBUG_RET( 1, "ssl_read_record", ret );
            return( ret );
        }

        if( ssl->in_msgtype != SSL_MSG_HANDSHAKE )
        {
            SSL_DEBUG_MSG( 1, ( "bad server hello done message" ) );
            return( POLARSSL_ERR_SSL_UNEXPECTED_MESSAGE );
        }
    }

    if( ssl->in_hslen  != 4 ||
        ssl->in_msg[0] != SSL_HS_SERVER_HELLO_DONE )
    {
        SSL_DEBUG_MSG( 1, ( "bad server hello done message" ) );
        return( POLARSSL_ERR_SSL_BAD_HS_SERVER_HELLO_DONE );
    }

    ssl->state++;

    SSL_DEBUG_MSG( 2, ( "<= parse server hello done" ) );

    return( 0 );
}

static int ssl_write_client_key_exchange( ssl_context *ssl )
{
    int ret, i, n;

    SSL_DEBUG_MSG( 2, ( "=> write client key exchange" ) );

    if( ssl->session->cipher == SSL_EDH_RSA_DES_168_SHA ||
        ssl->session->cipher == SSL_EDH_RSA_AES_256_SHA ||
	ssl->session->cipher == SSL_EDH_RSA_CAMELLIA_256_SHA)
    {
        SSL_DEBUG_MSG( 1, ( "support for dhm in not available" ) );
        return( POLARSSL_ERR_SSL_FEATURE_UNAVAILABLE );
    }
    else
    {
        /*
         * RSA key exchange -- send rsa_public_EX(pkcs1 v1.5(premaster))
         */
        ssl->premaster[0] = (unsigned char) ssl->max_major_ver;
        ssl->premaster[1] = (unsigned char) ssl->max_minor_ver;
        ssl->pmslen = 48;

        for( i = 2; i < ssl->pmslen; i++ )
            ssl->premaster[i] = (unsigned char) ssl->f_rng( ssl->p_rng );

        i = 4;
        n = ssl->peer_cert->rsa.len;

        if( ssl->minor_ver != SSL_MINOR_VERSION_0 )
        {
            i += 2;
            ssl->out_msg[4] = (unsigned char)( n >> 8 );
            ssl->out_msg[5] = (unsigned char)( n      );
        }

        ret = rsa_pkcs1_encrypt_EX( &ssl->peer_cert->rsa, RSA_PUBLIC,
                                  ssl->pmslen, ssl->premaster,
                                  ssl->out_msg + i );
        if( ret != 0 )
        {
            SSL_DEBUG_RET( 1, "rsa_pkcs1_encrypt_EX", ret );
            return( ret );
        }
    }

    ssl_derive_keys( ssl );

    ssl->out_msglen  = i + n;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CLIENT_KEY_EXCHANGE;

    ssl->state++;

    if( ( ret = ssl_write_record( ssl ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "ssl_write_record", ret );
        return( ret );
    }

    SSL_DEBUG_MSG( 2, ( "<= write client key exchange" ) );

    return( 0 );
}

static int ssl_write_certificate_verify( ssl_context *ssl )
{
    int ret, n;
    unsigned char hash[36];

    SSL_DEBUG_MSG( 2, ( "=> write certificate verify" ) );

    if( ssl->client_auth == 0 )
    {
        SSL_DEBUG_MSG( 2, ( "<= skip write certificate verify" ) );
        ssl->state++;
        return( 0 );
    }

    if( ssl->rsa_key == NULL )
    {
        SSL_DEBUG_MSG( 1, ( "got no private key" ) );
        return( POLARSSL_ERR_SSL_PRIVATE_KEY_REQUIRED );
    }

    /*
     * Make an RSA signature of the handshake digests
     */
    ssl_calc_verify( ssl, hash );

    n = ssl->rsa_key->len;
    ssl->out_msg[4] = (unsigned char)( n >> 8 );
    ssl->out_msg[5] = (unsigned char)( n      );

    if( ( ret = rsa_pkcs1_sign_EX( ssl->rsa_key, RSA_PRIVATE, SIG_RSA_RAW,
                                36, hash, ssl->out_msg + 6 ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "rsa_pkcs1_sign_EX", ret );
        return( ret );
    }

    ssl->out_msglen  = 6 + n;
    ssl->out_msgtype = SSL_MSG_HANDSHAKE;
    ssl->out_msg[0]  = SSL_HS_CERTIFICATE_VERIFY;

    ssl->state++;

    if( ( ret = ssl_write_record( ssl ) ) != 0 )
    {
        SSL_DEBUG_RET( 1, "ssl_write_record", ret );
        return( ret );
    }

    SSL_DEBUG_MSG( 2, ( "<= write certificate verify" ) );

    return( 0 );
}

/*
 * SSL handshake -- client side
 */
int ssl_handshake_client( ssl_context *ssl )
{
    int ret = 0;

    SSL_DEBUG_MSG( 2, ( "=> handshake client" ) );

    while( ssl->state != SSL_HANDSHAKE_OVER )
    {
        SSL_DEBUG_MSG( 2, ( "client state: %d", ssl->state ) );

        if( ( ret = ssl_flush_output( ssl ) ) != 0 )
            break;

        switch( ssl->state )
        {
            case SSL_HELLO_REQUEST:
                ssl->state = SSL_CLIENT_HELLO;
                break;

            /*
             *  ==>   ClientHello
             */
            case SSL_CLIENT_HELLO:
                ret = ssl_write_client_hello( ssl );
                break;

            /*
             *  <==   ServerHello
             *        Certificate
             *      ( ServerKeyExchange  )
             *      ( CertificateRequest )
             *        ServerHelloDone
             */
            case SSL_SERVER_HELLO:
                ret = ssl_parse_server_hello( ssl );
                break;

            case SSL_SERVER_CERTIFICATE:
                ret = ssl_parse_certificate( ssl );
                break;

            case SSL_SERVER_KEY_EXCHANGE:
                ret = ssl_parse_server_key_exchange( ssl );
                break;

            case SSL_CERTIFICATE_REQUEST:
                ret = ssl_parse_certificate_request( ssl );
                break;

            case SSL_SERVER_HELLO_DONE:
                ret = ssl_parse_server_hello_done( ssl );
                break;

            /*
             *  ==> ( Certificate/Alert  )
             *        ClientKeyExchange
             *      ( CertificateVerify  )
             *        ChangeCipherSpec
             *        Finished
             */
            case SSL_CLIENT_CERTIFICATE:
                ret = ssl_write_certificate( ssl );
                break;

            case SSL_CLIENT_KEY_EXCHANGE:
                ret = ssl_write_client_key_exchange( ssl );
                break;

            case SSL_CERTIFICATE_VERIFY:
                ret = ssl_write_certificate_verify( ssl );
                break;

            case SSL_CLIENT_CHANGE_CIPHER_SPEC:
                ret = ssl_write_change_cipher_spec( ssl );
                break;

            case SSL_CLIENT_FINISHED:
                ret = ssl_write_finished( ssl );
                break;

            /*
             *  <==   ChangeCipherSpec
             *        Finished
             */
            case SSL_SERVER_CHANGE_CIPHER_SPEC:
                ret = ssl_parse_change_cipher_spec( ssl );
                break;

            case SSL_SERVER_FINISHED:
                ret = ssl_parse_finished( ssl );
                break;

            case SSL_FLUSH_BUFFERS:
                SSL_DEBUG_MSG( 2, ( "handshake: done" ) );
                ssl->state = SSL_HANDSHAKE_OVER;
                break;

            default:
                SSL_DEBUG_MSG( 1, ( "invalid state %d", ssl->state ) );
                return( POLARSSL_ERR_SSL_BAD_INPUT_DATA );
        }

        if( ret != 0 )
            break;
    }

    SSL_DEBUG_MSG( 2, ( "<= handshake client" ) );

    return( ret );
}

#endif
#endif//#ifdef _SUPPORT_SSL
