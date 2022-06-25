/* user_settings.h
 *
 * Copyright (C) 2006-2017 wolfSSL Inc.  All rights reserved.
 * Additions Copyright 2018 Espressif Systems (Shanghai) PTE LTD.
 *
 * This file is part of wolfSSL.
 *
 * Contact licensing@wolfssl.com with any questions or comments.
 *
 * http://www.wolfssl.com
 */


#ifndef __USER_SETTINGS_H__
#define __USER_SETTINGS_H__

//#define WOLFSSL_LWIP
//#define FREERTOS

#define NO_WRITEV
#define NO_WOLFSSL_DIR
#ifndef NO_INLINE
#define NO_INLINE
#endif
#define NO_WOLFSSL_MEMORY
#define HAVE_PK_CALLBACKS
#define WOLFSSL_KEY_GEN
#define WOLFSSL_RIPEMD
#define USE_WOLFSSL_IO
#define WOLFSSL_STATIC_RSA
#define NO_DH
#define NO_MD4
#define NO_DES3
#define NO_DSA
#define NO_PSK
#define NO_PWDBASED
#define NO_RC4
#define NO_RABBIT
#define NO_HC128
#define WC_NO_HARDEN
#define WOLFSSL_TYPES
#define NO_FILESYSTEM
//#define WOLFSSL_ALT_CERT_CHAINS
//#define WOLFSSL_ALLOW_TLSV10

#define WOLFSSL_SMALL_STACK

#define NO_SESSION_CACHE
#define NO_WOLFSSL_SERVER
#define NO_ERROR_STRINGS
#define SINGLE_THREADED

#define WOLFSSL_USER_IO

//#define DEBUG_WOLFSSL
//#define WOLFSSL_LOG_PRINTF

//#define NO_ASN_TIME

#define XTIME time
#define SSL_CTX_use_certificate_ASN1(ctx,len,buf) wolfSSL_CTX_use_certificate_buffer(ctx,buf,len,WOLFSSL_FILETYPE_PEM)
#define SSL_CTX_use_PrivateKey_ASN1(type,ctx,buf,len) wolfSSL_CTX_use_PrivateKey_buffer(ctx,buf,len, WOLFSSL_FILETYPE_PEM)
#define SSL_CTX_load_verify_buffer(ctx,buf,len) wolfSSL_CTX_load_verify_buffer(ctx,buf,len, WOLFSSL_FILETYPE_PEM)

#ifdef WOLFSSL_TYPES
    #ifndef byte
        typedef unsigned char  byte;
    #endif
    typedef unsigned short word16;
    typedef unsigned int   word32;
    typedef byte           word24[3]; 
#endif

#ifndef CUSTOM_RAND_GENERATE_BLOCK
    /* To use define the following:*/
    //#define CUSTOM_RAND_GENERATE_BLOCK os_get_random
    #include "plat_osl.h"
    #define CUSTOM_RAND_GENERATE_BLOCK osl_get_random
#endif

#endif
