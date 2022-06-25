#pragma once

#ifndef _MSC_VER
#define _CONFIG_WOLFSSL
#endif

#ifdef _CONFIG_WOLFSSL
#include "wolfssl/ssl.h"
#include <cyassl/ssl.h>
#endif

#if defined OPENSSL_EXTRA
//#error OPENSSL_EXTRA
#else
//#error not OPENSSL_EXTRA
#endif

#if defined _CONFIG_WOLFSSL
namespace Bear {
namespace Core {
namespace Net {
using namespace Bear::Core;
using namespace std;

//XiongWanPing 2022.06.23
//SslFilter仿ZLMediaKit SSL_Box,目前只验证过clientMode,还没有用到serverMode为true的情况
//嵌入式t21的linux flash空间很小,用openssl是不可能的,用wolfssl后剩余空间都只剩下68KB了,还需要精简其他组件
class SslFilter {
public:
	SslFilter();
	~SslFilter();

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
	void init();
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
