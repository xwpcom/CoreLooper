#include "stdafx.h"
#include "tlsproxy.h"

namespace Bear {
namespace Core {
namespace Crypt {

#define DFL_DEBUG_LEVEL         0
#define DFL_FORCE_CIPHER        0
#define DFL_MODE                0
#define DFL_AUTHENTICATION      1

#define MODE_SSL_TLS            0
#define MODE_STARTTLS           0

struct options
{
	string server_name;    /* hostname of the server (client only)     */
	int debug_level;            /* level of debugging                       */
	int mode;                   /* SSL/TLS (0) or STARTTLS (1)              */
	string user_name;      /* username to use for authentication       */
	string user_pwd;       /* password to use for authentication       */
	int force_ciphersuite[2];   /* protocol/ciphersuite to use, or all      */
} opt;

TlsProxy::TlsProxy()
{
	Clear();
}

TlsProxy::~TlsProxy()
{

}

void TlsProxy::Clear()
{
#define CLEAR(x) memset(&x,0,sizeof(x))
	CLEAR(server_fd);
	CLEAR(entropy);
	CLEAR(ctr_drbg);
	CLEAR(ssl);
	CLEAR(conf);
#ifdef MBEDTLS_X509_CRT_PARSE_C
	CLEAR(cacert);
	CLEAR(clicert);
#endif
	CLEAR(pkey);
	CLEAR(buf);
}

int TlsProxy::do_handshake(mbedtls_ssl_context *ssl)
{
	int ret;
	uint32_t flags;
	unsigned char buf[1024];
	memset(buf, 0, sizeof(buf));

	while ((ret = mbedtls_ssl_handshake(ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
#if defined(MBEDTLS_ERROR_C)
			mbedtls_strerror(ret, (char *)buf, 1024);
#endif
			DT(" failed  ! mbedtls_ssl_handshake returned %d: %s", ret, buf);
			return(-1);
		}
	}

	//5. Verify the server certificate
	DT("  . Verifying peer X.509 certificate...");

	/* In real life, we probably want to bail out when ret != 0 */
	if ((flags = mbedtls_ssl_get_verify_result(ssl)) != 0)
	{
		DT(" failed");

#ifdef MBEDTLS_X509_CRT_PARSE_C
		char vrfy_buf[512];
		mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
		DT("%s", vrfy_buf);
#endif
	}
	else
	{
		DT(" ok");
	}

	DT("  . Peer certificate information    ...");
#ifdef MBEDTLS_X509_CRT_PARSE_C
	mbedtls_x509_crt_info((char *)buf, sizeof(buf) - 1, "      ", mbedtls_ssl_get_peer_cert(ssl));
	DT("%s", buf);
#endif

	return 0;
}

int TlsProxy::write_ssl_data(unsigned char *buf, size_t len)
{
	int ret;

	DT("%s", buf);
	while (len && (ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			DT(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
			return -1;
		}
	}

	return(0);
}

int TlsProxy::write_ssl_and_get_response(unsigned char *buf, size_t len)
{
	int ret;
	unsigned char data[128];
	char code[4];
	size_t i, idx = 0;

	DT(("C=>S %s"), buf);

	while (len && (ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			DT(" failed! mbedtls_ssl_write returned %d", ret);
			return -1;
		}
	}

	do
	{
		len = sizeof(data) - 1;
		memset(data, 0, sizeof(data));
		ret = mbedtls_ssl_read(&ssl, data, len);

		if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;

		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
			return -1;

		if (ret <= 0)
		{
			DT("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
			return -1;
		}

		DT("S=>C %s", data);

		len = ret;
		for (i = 0; i < len; i++)
		{
			if (data[i] != '\n')
			{
				if (idx < 4)
					code[idx++] = data[i];
				continue;
			}

			if (idx == 4 && code[0] >= '0' && code[0] <= '9' && code[3] == ' ')
			{
				code[3] = '\0';
				return atoi(code);
			}

			idx = 0;
		}
	} while (1);
}

int TlsProxy::write_and_get_response(unsigned char *buf, size_t len)
{
	mbedtls_net_context *sock_fd = &server_fd;

	int ret;
	unsigned char data[128];
	char code[4];
	size_t i, idx = 0;

	DT("%s", buf);
	if (len && (ret = mbedtls_net_send(sock_fd, buf, len)) <= 0)
	{
		DT(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
		return -1;
	}

	do
	{
		len = sizeof(data) - 1;
		memset(data, 0, sizeof(data));
		ret = mbedtls_net_recv(sock_fd, data, len);

		if (ret <= 0)
		{
			DT("failed\n  ! read returned %d\n\n", ret);
			return -1;
		}

		data[len] = '\0';
		DT("%s", data);
		len = ret;
		for (i = 0; i < len; i++)
		{
			if (data[i] != '\n')
			{
				if (idx < 4)
					code[idx++] = data[i];
				continue;
			}

			if (idx == 4 && code[0] >= '0' && code[0] <= '9' && code[3] == ' ')
			{
				code[3] = '\0';
				return atoi(code);
			}

			idx = 0;
		}
	} while (1);
}

int TlsProxy::Init(SOCKET& s, string smtpServer, int port, string sender, string password)
{
	mbedtls_net_init(&server_fd);
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	memset(&buf, 0, sizeof(buf));
#ifdef MBEDTLS_X509_CRT_PARSE_C
	mbedtls_x509_crt_init(&cacert);
	mbedtls_x509_crt_init(&clicert);
#endif
	mbedtls_pk_init(&pkey);
	mbedtls_ctr_drbg_init(&ctr_drbg);

	opt.server_name = smtpServer;
	opt.debug_level = DFL_DEBUG_LEVEL;
	opt.mode = DFL_MODE;
	opt.user_name = sender;
	opt.user_pwd = password;
	opt.force_ciphersuite[0] = DFL_FORCE_CIPHER;

	//Initialize the RNG and the session data
	DT("Seeding the random number generator...");

	mbedtls_entropy_init(&entropy);
	const char *pers = "CSmtp";
	int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers));
	if (ret)
	{
		DT(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
	}

	server_fd.fd = (int)s;
	mbedtls_net_set_nonblock(&server_fd);


	if ((ret = mbedtls_ssl_config_defaults(&conf,
		MBEDTLS_SSL_IS_CLIENT,
		MBEDTLS_SSL_TRANSPORT_STREAM,
		MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		DT(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
		return -1;
	}

	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	mbedtls_ssl_conf_dbg(&conf, Dump, stdout);

	if (opt.force_ciphersuite[0] != DFL_FORCE_CIPHER)
		mbedtls_ssl_conf_ciphersuites(&conf, opt.force_ciphersuite);

#ifdef MBEDTLS_X509_CRT_PARSE_C
	mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
	if ((ret = mbedtls_ssl_conf_own_cert(&conf, &clicert, &pkey)) != 0)
	{
		DT(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret);
		return -1;
	}
#endif

	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
	{
		DT(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
		return -1;
	}

#ifdef MBEDTLS_X509_CRT_PARSE_C
	//if ((ret = mbedtls_ssl_set_hostname(&ssl, opt.server_name.c_str())) != 0)
	{
		//DT(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
		//return -1;
	}
#endif

	mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	char hostname[32];
	gethostname(hostname, sizeof(hostname) - 1);

	int len = 0;

	if (opt.mode == MODE_SSL_TLS)
	{
		if (do_handshake(&ssl) != 0)
			return -1;

		DT("  > Get header from server:");


		ret = write_ssl_and_get_response(buf, 0);
		if (ret < 200 || ret > 299)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		DT(" ok\n");

		DT("  > Write EHLO to server:");


		len = sprintf((char *)buf, "EHLO %s\r\n", hostname);
		ret = write_ssl_and_get_response(buf, len);
		if (ret < 200 || ret > 299)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}
	}
	else
	{
		ret = write_and_get_response(buf, 0);
		if (ret < 200 || ret > 299)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		DT("  > Write EHLO to server:");

		len = sprintf((char *)buf, "EHLO %s\r\n", hostname);
		ret = write_and_get_response(buf, len);
		if (ret < 200 || ret > 299)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		DT("  > Write STARTTLS to server:");
		len = sprintf((char *)buf, "STARTTLS\r\n");
		ret = write_and_get_response(buf, len);
		if (ret < 200 || ret > 299)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		if (do_handshake(&ssl) != 0)
			return -1;
	}

	{
		DT("Write AUTH LOGIN to server:");


		len = sprintf((char *)buf, "AUTH LOGIN\r\n");
		ret = write_ssl_and_get_response(buf, len);
		if (ret < 200 || ret > 399)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		DT("Write username to server: %s", opt.user_name.c_str());

		unsigned char base[1024];
		size_t n = 0;
		ret = mbedtls_base64_encode(base, sizeof(base), &n, (const unsigned char *)opt.user_name.c_str(), opt.user_name.length());

		if (ret != 0) {
			DT(" failed\n  ! mbedtls_base64_encode returned %d\n\n", ret);
			return -1;
		}
		len = sprintf((char *)buf, "%s\r\n", base);
		ret = write_ssl_and_get_response(buf, len);
		if (ret < 300 || ret > 399)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

		DT("  > Write password to server: %s", opt.user_pwd.c_str());
		ret = mbedtls_base64_encode(base, sizeof(base), &n, (const unsigned char *)opt.user_pwd.c_str(), opt.user_pwd.length());

		if (ret != 0) {
			DT(" failed\n  ! mbedtls_base64_encode returned %d\n\n", ret);
			return -1;
		}
		len = sprintf((char *)buf, "%s\r\n", base);
		ret = write_ssl_and_get_response(buf, len);
		if (ret < 200 || ret > 399)
		{
			DT(" failed\n  ! server responded with %d\n\n", ret);
			return -1;
		}

	}

	return ret;
}

void TlsProxy::UnInit()
{
	mbedtls_net_free(&server_fd);
#ifdef MBEDTLS_X509_CRT_PARSE_C
	mbedtls_x509_crt_free(&clicert);
	mbedtls_x509_crt_free(&cacert);
#endif
	mbedtls_pk_free(&pkey);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	Clear();
}

void TlsProxy::Dump(void *ctx, int level, const char *file, int line, const char *str)
{
	((void)level);

	//mbedtls_fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
	fflush((FILE *)ctx);
}


}
}
}
