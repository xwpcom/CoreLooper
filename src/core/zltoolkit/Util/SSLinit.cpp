#include "stdafx.h"
using namespace Bear::Core;
static const char* TAG = "sslBox";
static bool mVerbose = true;
/*
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 */
#include <string.h>
#include "SSLBox.h"
#include "util.h"
#include "onceToken.h"
#include "SSLUtil.h"

#if defined(ENABLE_OPENSSL)
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/bio.h>
#include <openssl/ossl_typ.h>

#if defined(_WIN32)
#if defined(_WIN64)

 //64bit
#if !defined(NDEBUG)
#pragma  comment (lib,"libssl64MDd")
#pragma  comment (lib,"libcrypto64MDd")
#else
#pragma  comment (lib,"libssl64MD")
#pragma  comment (lib,"libcrypto64MD")
#endif // !defined(NDEBUG)

#else

 //32 bit
#if !defined(NDEBUG)
#pragma  comment (lib,"libssl32MDd")
#pragma  comment (lib,"libcrypto32MDd")

#else
#pragma  comment (lib,"libssl32MD")
#pragma  comment (lib,"libcrypto32MD")
#endif // !defined(NDEBUG)

#endif //defined(_WIN64)
#endif // defined(_WIN32)

#endif //defined(ENABLE_OPENSSL)

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
//openssl版本是否支持sni
#define SSL_ENABLE_SNI
#endif

namespace toolkit {


static bool s_ingroleSsl = true;

SSL_Initor& SSL_Initor::Instance() {
	static SSL_Initor obj;
	return obj;
}

void SSL_Initor::ignoreInvalidCertificate(bool ignore) {
	s_ingroleSsl = ignore;
}

SSL_Initor::SSL_Initor() {
#if defined(ENABLE_OPENSSL)
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_ciphers();

#if defined  _MSC_VER_DEBUG
	{
		//LogV(TAG, "openssl version=%08x", OPENSSL_VERSION_NUMBER);
	}
#endif

	OpenSSL_add_all_algorithms();
	CRYPTO_set_locking_callback([](int mode, int n,
		const char* file, int line) {
			static mutex* s_mutexes = new mutex[CRYPTO_num_locks()];
			static onceToken token(nullptr, []() {
				delete[] s_mutexes;
				});
			if (mode & CRYPTO_LOCK) {
				s_mutexes[n].lock();
			}
			else {
				s_mutexes[n].unlock();
			}
		});

	CRYPTO_set_id_callback([]() ->unsigned long {
	#if !defined(_WIN32)
		return (unsigned long)pthread_self();
	#else
		return (unsigned long)GetCurrentThreadId();
	#endif
		});

	setContext("", SSLUtil::makeSSLContext(nullptr, nullptr, false), false);
	setContext("", SSLUtil::makeSSLContext(nullptr, nullptr, true), true);
#endif //defined(ENABLE_OPENSSL)
}

SSL_Initor::~SSL_Initor() {
#if defined(ENABLE_OPENSSL)
	EVP_cleanup();
	ERR_free_strings();
	ERR_clear_error();
	ERR_remove_state(0);
	CRYPTO_set_locking_callback(NULL);
	//sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
	CRYPTO_cleanup_all_ex_data();
	CONF_modules_unload(1);
	CONF_modules_free();
#endif //defined(ENABLE_OPENSSL)
}

bool SSL_Initor::loadCertificate(const string& pem_or_p12, bool serverMode, const string& passwd, bool isFile, bool isDefault) {
	return loadCertificate(SSLUtil::loadPublicKey(pem_or_p12, passwd, isFile).get(),
		SSLUtil::loadPrivateKey(pem_or_p12, passwd, isFile).get(),
		serverMode, isDefault);
}

bool SSL_Initor::loadCertificate(X509* public_key, EVP_PKEY* private_key, bool serverMode, bool isDefault) {
	return setContext(SSLUtil::getServerName(public_key),
		SSLUtil::makeSSLContext(public_key, private_key, serverMode),
		serverMode, isDefault);
}

int SSL_Initor::findCertificate(SSL* ssl, int* ad, void* arg) {
#if !defined(ENABLE_OPENSSL) || !defined(SSL_ENABLE_SNI)
	return 0;
#else
	if (!ssl) {
		return SSL_TLSEXT_ERR_ALERT_FATAL;
	}

	SSL_CTX* ctx = NULL;
	static auto& ref = SSL_Initor::Instance();
	const char* vhost = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

	if (vhost && vhost[0] != '\0') {
		//从map中找到vhost对应的SSL_CTX
		ctx = ref.getSSLCtx(vhost, (bool)(arg)).get();
	}
	else {
		//选一个默认的SSL_CTX
		ctx = ref.getSSLCtx("", (bool)(arg)).get();
		if (ctx) {
			DebugL << "client does not specify host, select default certificate of host: " << ref.defaultVhost((bool)(arg));
		}
		else {
			vhost = "default host";
		}
	}

	if (!ctx) {
		//未找到对应的证书
		DebugL << "can not find any certificate of host:" << vhost;
		return SSL_TLSEXT_ERR_ALERT_FATAL;
	}

	SSL_set_SSL_CTX(ssl, ctx);
	return SSL_TLSEXT_ERR_OK;
#endif
}

bool SSL_Initor::setContext(const string& vhost, const shared_ptr<SSL_CTX>& ctx, bool serverMode, bool isDefault) {
	if (!ctx) {
		return false;
	}
	setupCtx(ctx.get());

#if defined(ENABLE_OPENSSL)
	if (vhost.empty()) {
		_ctx_empty[serverMode] = ctx;

	#ifdef SSL_ENABLE_SNI
		if (serverMode) {
			SSL_CTX_set_tlsext_servername_callback(ctx.get(), findCertificate);
			SSL_CTX_set_tlsext_servername_arg(ctx.get(), (void*)serverMode);
		}
	#endif // SSL_ENABLE_SNI

	}
	else {
		_ctxs[serverMode][vhost] = ctx;
		if (isDefault) {
			_default_vhost[serverMode] = vhost;
		}
		DebugL << "add certificate of: " << vhost;
	}
	return true;
#else
	WarnL << "ENABLE_OPENSSL宏未启用,openssl相关功能将无效!";
	return false;
#endif //defined(ENABLE_OPENSSL)
}


void SSL_Initor::setupCtx(SSL_CTX* ctx) {
#if defined(ENABLE_OPENSSL)
	//加载默认信任证书
	SSLUtil::loadDefaultCAs(ctx);

	//SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
	//https://myssl.com/bear.jjyip.com?domain=bear.jjyip.com&port=443&status=success
	SSL_CTX_set_cipher_list(ctx,
		"ECDHE-RSA-AES128-GCM-SHA256:ECDHE:ECDH:AES:HIGH:!NULL:!aNULL:!MD5:!ADH:!RC4:!DH:!DHE"
	);

	SSL_CTX_set_verify_depth(ctx, 9);
	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, [](int ok, X509_STORE_CTX* pStore) {
		if (!ok) {
			int depth = X509_STORE_CTX_get_error_depth(pStore);
			int err = X509_STORE_CTX_get_error(pStore);
			std::string error(X509_verify_cert_error_string(err));
			WarnL << depth << " " << error;
		}
		if (s_ingroleSsl) {
			ok = 1;
		}
		return ok;
		});

#endif //defined(ENABLE_OPENSSL)
}

shared_ptr<SSL> SSL_Initor::makeSSL(bool serverMode) {
#if defined(ENABLE_OPENSSL)
#ifdef SSL_ENABLE_SNI
	//openssl 版本支持SNI
	return SSLUtil::makeSSL(_ctx_empty[serverMode].get());
#else
	//openssl 版本不支持SNI，选择默认证书
	return SSLUtil::makeSSL(getSSLCtx("", serverMode).get());
#endif//SSL_CTRL_SET_TLSEXT_HOSTNAME
#else
	return nullptr;
#endif //defined(ENABLE_OPENSSL)
}

bool SSL_Initor::trustCertificate(X509* cer, bool serverMode) {
	return SSLUtil::trustCertificate(_ctx_empty[serverMode].get(), cer);
}

bool SSL_Initor::trustCertificate(const string& pem_p12_cer, bool serverMode, const string& passwd, bool isFile) {
	return trustCertificate(SSLUtil::loadPublicKey(pem_p12_cer, passwd, isFile).get(), serverMode);
}

std::shared_ptr<SSL_CTX> SSL_Initor::getSSLCtx(const string& vhost, bool serverMode) {
	if (!serverMode) {
		return _ctx_empty[serverMode];
	}

	if (vhost.empty()) {
		return _ctxs[serverMode][_default_vhost[serverMode]];
	}
	return _ctxs[serverMode][vhost];
}

string SSL_Initor::defaultVhost(bool serverMode) {
	return _default_vhost[serverMode];
}

}
