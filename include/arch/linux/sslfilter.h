#pragma once

#ifndef _MSC_VER
#define _CONFIG_WOLFSSL
#endif

#ifdef _CONFIG_WOLFSSL
#include "wolfssl/ssl.h"
#include <cyassl/ssl.h>

namespace Bear {
namespace Core {
namespace Net {
using namespace Bear::Core;
using namespace std;

#define BIO_s_mem                       wolfSSL_BIO_s_mem
#define BIO_new                         wolfSSL_BIO_new
#define BIO_free                        wolfSSL_BIO_free
#define BIO_method_type                 wolfSSL_BIO_method_type
#define BIO_set_ssl                     wolfSSL_BIO_set_ssl
#define BIO_get_ssl                     wolfSSL_BIO_get_ssl
#define BIO_new_ssl_connect             wolfSSL_BIO_new_ssl_connect
#define BIO_set_conn_hostname           wolfSSL_BIO_set_conn_hostname

#define SSL_new                         wolfSSL_new
#define SSL_set_fd                      wolfSSL_set_fd
#define SSL_get_fd                      wolfSSL_get_fd
#define SSL_connect                     wolfSSL_connect
#define SSL_clear                       wolfSSL_clear
#define SSL_set_bio                     wolfSSL_set_bio
#define SSL_do_handshake                wolfSSL_SSL_do_handshake
#define SSL_set_shutdown                wolfSSL_set_shutdown
#define SSL_set_session_id_context      wolfSSL_set_session_id_context
#define SSL_set_connect_state           wolfSSL_set_connect_state
#define SSL_set_accept_state            wolfSSL_set_accept_state


typedef WOLFSSL          SSL;


//XiongWanPing 2022.06.23
//SslFilter仿ZLMediaKit SSL_Box,目前只验证过clientMode,还没有用到serverMode为true的情况
//嵌入式t21的linux flash空间很小,用openssl是不可能的,用wolfssl后剩余空间都只剩下68KB了,还需要精简其他组件
class SslFilter {
public:
	SslFilter();
	~SslFilter();

	void init();
	void onConnect();
	/**
	 * 收到密文后，调用此函数解密
	 * @param buffer 收到的密文数据
	 */
	void onRecv(shared_ptr<ByteBuffer> buffer);

	/**
	 * 需要加密明文调用此函数
	 * @param buffer 需要加密的明文数据
	 */
	void onSend(shared_ptr<ByteBuffer> buffer);

	/**
	 * 设置解密后获取明文的回调
	 * @param fun 回调对象
	 */
	void setOnDecData(function<void(shared_ptr<ByteBuffer>)> fun) {
		_onDec = fun;
	}

	/**
	 * 设置加密后获取密文的回调
	 * @param fun 回调对象
	 */
	void setOnEncData(function<void(shared_ptr<ByteBuffer>)> fun) {
		_onEnc = fun;
	}

	void shutdown();

	void flush();

	bool setHost(const char* host);
protected:
	void flushWriteBio();
	void flushReadBio();

	bool mServerMode = false;
	bool mSendHandshake = false;
	shared_ptr<WOLFSSL> mSSL;
	WOLFSSL_BIO* _read_bio = nullptr, * _write_bio = nullptr;
	function<void(shared_ptr<ByteBuffer>)> _onDec;
	function<void(shared_ptr<ByteBuffer>)> _onEnc;
	list<shared_ptr<ByteBuffer>> _bufferOut;
	int mBufSize = 32 * 1024;
};

}
}
}
#endif
