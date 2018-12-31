#pragma once

extern "C"
{
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#endif

#include "mbedtls/base64.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
}


namespace Bear {
namespace Core {
namespace Crypt {

class TlsProxy
{
public:
	TlsProxy();
	virtual ~TlsProxy();
	void Clear();
	
	int Init(SOCKET& s, string smtpServer, int port, string sender, string password);
	static int do_handshake(mbedtls_ssl_context *ssl);
	int write_ssl_data(unsigned char *buf, size_t len);
	int write_ssl_and_get_response(unsigned char *buf, size_t len);
	int write_and_get_response(unsigned char *buf, size_t len);
	void UnInit();

protected:
	static void Dump(void *ctx, int level, const char *file, int line, const char *str);

	mbedtls_net_context server_fd;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
#ifdef MBEDTLS_X509_CRT_PARSE_C
	mbedtls_x509_crt cacert;
	mbedtls_x509_crt clicert;
#endif
	mbedtls_pk_context pkey;
	unsigned char buf[1024];
};

}
}
}
