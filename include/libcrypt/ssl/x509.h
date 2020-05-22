#ifdef _SUPPORT_SSL
/**
 * \file x509.h
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
#ifndef POLARSSL_X509_H
#define POLARSSL_X509_H

#include "rsa.h"

/*
 * ASN1 Error codes
 *
 * These error codes will be OR'ed to X509 error codes for
 * higher error granularity.
 */
#define POLARSSL_ERR_ASN1_OUT_OF_DATA                      0x0014
#define POLARSSL_ERR_ASN1_UNEXPECTED_TAG                   0x0016
#define POLARSSL_ERR_ASN1_INVALID_LENGTH                   0x0018
#define POLARSSL_ERR_ASN1_LENGTH_MISMATCH                  0x001A
#define POLARSSL_ERR_ASN1_INVALID_DATA                     0x001C

/*
 * X509 Error codes
 */
#define POLARSSL_ERR_X509_FEATURE_UNAVAILABLE              -0x0020
#define POLARSSL_ERR_X509_CERT_INVALID_PEM                 -0x0040
#define POLARSSL_ERR_X509_CERT_INVALID_FORMAT              -0x0060
#define POLARSSL_ERR_X509_CERT_INVALID_VERSION             -0x0080
#define POLARSSL_ERR_X509_CERT_INVALID_SERIAL              -0x00A0
#define POLARSSL_ERR_X509_CERT_INVALID_ALG                 -0x00C0
#define POLARSSL_ERR_X509_CERT_INVALID_NAME                -0x00E0
#define POLARSSL_ERR_X509_CERT_INVALID_DATE                -0x0100
#define POLARSSL_ERR_X509_CERT_INVALID_PUBKEY              -0x0120
#define POLARSSL_ERR_X509_CERT_INVALID_SIGNATURE           -0x0140
#define POLARSSL_ERR_X509_CERT_INVALID_EXTENSIONS          -0x0160
#define POLARSSL_ERR_X509_CERT_UNKNOWN_VERSION             -0x0180
#define POLARSSL_ERR_X509_CERT_UNKNOWN_SIG_ALG             -0x01A0
#define POLARSSL_ERR_X509_CERT_UNKNOWN_PK_ALG              -0x01C0
#define POLARSSL_ERR_X509_CERT_SIG_MISMATCH                -0x01E0
#define POLARSSL_ERR_X509_CERT_VERIFY_FAILED               -0x0200
#define POLARSSL_ERR_X509_KEY_INVALID_PEM                  -0x0220
#define POLARSSL_ERR_X509_KEY_INVALID_VERSION              -0x0240
#define POLARSSL_ERR_X509_KEY_INVALID_FORMAT               -0x0260
#define POLARSSL_ERR_X509_KEY_INVALID_ENC_IV               -0x0280
#define POLARSSL_ERR_X509_KEY_UNKNOWN_ENC_ALG              -0x02A0
#define POLARSSL_ERR_X509_KEY_PASSWORD_REQUIRED            -0x02C0
#define POLARSSL_ERR_X509_KEY_PASSWORD_MISMATCH            -0x02E0
#define POLARSSL_ERR_X509_POINT_ERROR                      -0x0300
#define POLARSSL_ERR_X509_VALUE_TO_LENGTH                  -0x0320

/*
 * X509 Verify codes
 */
#define BADCERT_EXPIRED                 1
#define BADCERT_REVOKED                 2
#define BADCERT_CN_MISMATCH             4
#define BADCERT_NOT_TRUSTED             8
#define BADCRL_NOT_TRUSTED             16
#define BADCRL_EXPIRED                 32

/*
 * DER constants
 */
#define ASN1_BOOLEAN                 0x01
#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_OCTET_STRING            0x04
#define ASN1_NULL                    0x05
#define ASN1_OID                     0x06
#define ASN1_UTF8_STRING             0x0C
#define ASN1_SEQUENCE                0x10
#define ASN1_SET                     0x11
#define ASN1_PRINTABLE_STRING        0x13
#define ASN1_T61_STRING              0x14
#define ASN1_IA5_STRING              0x16
#define ASN1_UTC_TIME                0x17
#define ASN1_UNIVERSAL_STRING        0x1C
#define ASN1_BMP_STRING              0x1E
#define ASN1_PRIMITIVE               0x00
#define ASN1_CONSTRUCTED             0x20
#define ASN1_CONTEXT_SPECIFIC        0x80

/*
 * various object identifiers
 */
#define X520_COMMON_NAME                3
#define X520_COUNTRY                    6
#define X520_LOCALITY                   7
#define X520_STATE                      8
#define X520_ORGANIZATION              10
#define X520_ORG_UNIT                  11
#define PKCS9_EMAIL                     1

#define X509_OUTPUT_DER              0x01
#define X509_OUTPUT_PEM              0x02
#define PEM_LINE_LENGTH                72
#define X509_ISSUER                  0x01
#define X509_SUBJECT                 0x02

#define OID_X520                "\x55\x04"
#define OID_CN                  "\x55\x04\x03"
#define OID_PKCS1               "\x2A\x86\x48\x86\xF7\x0D\x01\x01"
#define OID_PKCS1_RSA           "\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01"
#define OID_PKCS1_RSA_SHA       "\x2A\x86\x48\x86\xF7\x0D\x01\x01\x05"
#define OID_PKCS9               "\x2A\x86\x48\x86\xF7\x0D\x01\x09"
#define OID_PKCS9_EMAIL         "\x2A\x86\x48\x86\xF7\x0D\x01\x09\x01"

/*
 * Structures for parsing X.509 certificates
 */
typedef struct _x509_buf
{
    int tag;
    int len;
    unsigned char *p;
}
x509_buf_EX;

typedef struct _x509_name
{
    x509_buf_EX oid;
    x509_buf_EX val;
    struct _x509_name *next;
}
x509_name_EX;

typedef struct _x509_time
{
    int year, mon, day;
    int hour, min, sec;
}
x509_time_EX;

typedef struct _x509_cert
{
    x509_buf_EX raw;
    x509_buf_EX tbs;

    int version;
    x509_buf_EX serial;
    x509_buf_EX sig_oid1;

    x509_buf_EX issuer_raw;
    x509_buf_EX subject_raw;

    x509_name_EX issuer;
    x509_name_EX subject;

    x509_time_EX valid_from;
    x509_time_EX valid_to;

    x509_buf_EX pk_oid;
    rsa_context_EX rsa;

    x509_buf_EX issuer_id;
    x509_buf_EX subject_id;
    x509_buf_EX v3_ext;

    int ca_istrue;
    int max_pathlen;

    x509_buf_EX sig_oid2;
    x509_buf_EX sig;

    struct _x509_cert *next; 
}
x509_cert_EX;

typedef struct _x509_crl_entry
{
    x509_buf_EX raw;

    x509_buf_EX serial;

    x509_time_EX revocation_date;

    x509_buf_EX entry_ext;

    struct _x509_crl_entry *next;
}
x509_crl_entry_EX;

typedef struct _x509_crl
{
    x509_buf_EX raw;
    x509_buf_EX tbs;

    int version;
    x509_buf_EX sig_oid1;

    x509_buf_EX issuer_raw;

    x509_name_EX issuer;

    x509_time_EX this_update;
    x509_time_EX next_update;

    x509_crl_entry_EX entry;

    x509_buf_EX crl_ext;

    x509_buf_EX sig_oid2;
    x509_buf_EX sig;

    struct _x509_crl *next; 
}
x509_crl_EX;

/*
 * Structures for writing X.509 certificates
 */
typedef struct _x509_node
{
    unsigned char *data;
    unsigned char *p;
    unsigned char *end;

    size_t len;
}
x509_node_EX;

typedef struct _x509_raw
{
    x509_node_EX raw;
    x509_node_EX tbs;

    x509_node_EX version;
    x509_node_EX serial;
    x509_node_EX tbs_signalg;
    x509_node_EX issuer;
    x509_node_EX validity;
    x509_node_EX subject;
    x509_node_EX subpubkey;

    x509_node_EX signalg;
    x509_node_EX sign;
}
x509_raw_EX;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Parse one or more certificates and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the certificate data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crt_EX( x509_cert_EX *chain, unsigned char *buf, int buflen );

/**
 * \brief          Load one or more certificates and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param path     filename to read the certificates from
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crtfile_EX( x509_cert_EX *chain, char *path );

/**
 * \brief          Parse one or more CRLs and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the CRL data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crl_EX( x509_crl_EX *chain, unsigned char *buf, int buflen );

/**
 * \brief          Load one or more CRLs and add them
 *                 to the chained list
 *
 * \param chain    points to the start of the chain
 * \param path     filename to read the CRLs from
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_crlfile_EX( x509_crl_EX *chain, char *path );

/**
 * \brief          Parse a private RSA key
 *
 * \param rsa      RSA context to be initialized
 * \param buf      input buffer
 * \param buflen   size of the buffer
 * \param pwd      password for decryption (optional)
 * \param pwdlen   size of the password
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_key_EX( rsa_context_EX *rsa,
                   unsigned char *buf, int buflen,
                   unsigned char *pwd, int pwdlen );

/**
 * \brief          Load and parse a private RSA key
 *
 * \param rsa      RSA context to be initialized
 * \param path     filename to read the private key from
 * \param pwd      password to decrypt the file (can be NULL)
 *
 * \return         0 if successful, or a specific X509 error code
 */
int x509parse_keyfile_EX( rsa_context_EX *rsa, char *path, char *password );

/**
 * \brief          Store the certificate DN in printable form into buf;
 *                 no more than size characters will be written.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param dn       The X509 name to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509parse_dn_gets_EX( char *buf, size_t size, x509_name_EX *dn );

/**
 * \brief          Returns an informational string about the
 *                 certificate.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param crt      The X509 certificate to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509parse_cert_info_EX( char *buf, size_t size, char *prefix, x509_cert_EX *crt );

/**
 * \brief          Returns an informational string about the
 *                 CRL.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param crt      The X509 CRL to represent
 *
 * \return         The amount of data written to the buffer, or -1 in
 *                 case of an error.
 */
int x509parse_crl_info_EX( char *buf, size_t size, char *prefix, x509_crl_EX *crl );

/**
 * \brief          Check a given x509_time_EX against the system time and check
 *                 if it is valid.
 *
 * \param time     x509_time_EX to check
 *
 * \return         Return 0 if the x509_time_EX is still valid,
 *                 or 1 otherwise.
 */
int x509parse_time_expired_EX( x509_time_EX *time );

/**
 * \brief          Verify the certificate signature
 *
 * \param crt      a certificate to be verified
 * \param trust_ca the trusted CA chain
 * \param ca_crl   the CRL chain for trusted CA's
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 *
 * \return         0 if successful or POLARSSL_ERR_X509_SIG_VERIFY_FAILED,
 *                 in which case *flags will have one or more of
 *                 the following values set:
 *                      BADCERT_EXPIRED --
 *                      BADCERT_REVOKED --
 *                      BADCERT_CN_MISMATCH --
 *                      BADCERT_NOT_TRUSTED
 *
 * \note           TODO: add two arguments, depth and crl
 */
int x509parse_verify_EX( x509_cert_EX *crt,
                      x509_cert_EX *trust_ca,
                      x509_crl_EX *ca_crl,
                      char *cn, int *flags );

/**
 * \brief          Unallocate all certificate data
 *
 * \param crt      Certificate chain to free
 */
void x509_free_EX( x509_cert_EX *crt );

/**
 * \brief          Unallocate all CRL data
 *
 * \param crt      CRL chain to free
 */
void x509_crl_free_EX( x509_crl_EX *crl );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int x509_self_test_EX( int verbose );

#ifdef __cplusplus
}
#endif

#endif /* x509.h */
#endif//#ifdef _SUPPORT_SSL
