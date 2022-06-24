#include "stdafx.h"
#include "sslfilter.h"

#if defined _CONFIG_WOLFSSL

namespace Bear {
namespace Core {
namespace Net {

#define SSL_ENABLE_SNI
using namespace Bear::Core;
static const char* TAG = "SslFilter";
static bool mVerbose = true;//为true时打印调试信息

SslFilter::~SslFilter() {}

SslFilter::SslFilter(bool serverMode, int buffSize) {
#if defined(ENABLE_OPENSSL)
	_read_bio = BIO_new(BIO_s_mem());
	mServerMode = serverMode;
	{
		WOLFSSL_CTX* ctx = NULL;
		ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
		{
			auto ret = wolfSSL_CTX_load_verify_locations(ctx, "D:/iot.jjyip.com.p12"
				//, 0
				, R"(D:\os\wolfssl\certs\external)"
			);
			LogV(TAG, "wolfSSL_CTX_load_verify_locations ret=%d", ret);
		}

		auto ssl = SSL_new(ctx);
		mSSL = shared_ptr<SSL>(ssl, [](SSL* ptr) {
			SSL_free(ptr);
			});

	}
	if (mSSL) {
		_write_bio = BIO_new(BIO_s_mem());
		SSL_set_bio(mSSL.get(), _read_bio, _write_bio);
		mServerMode ? SSL_set_accept_state(mSSL.get()) : SSL_set_connect_state(mSSL.get());
	}
	else {
		LogW(TAG, "ssl disabled!");
	}
	mSendHandshake = false;
	mBufSize = buffSize;
#endif 
}

void SslFilter::shutdown() {
#if defined(ENABLE_OPENSSL)
	_bufferOut.clear();
	int ret = SSL_shutdown(mSSL.get());
	if (ret != 1) {
		LogW(TAG, "SSL shutdown failed:");// << SSLUtil::getLastError();
	}
	else {
		flush();
	}
#endif 
}

void SslFilter::onRecv(shared_ptr<ByteBuffer> buffer) {
	if (!buffer->length()) {
		return;
	}
	if (!mSSL) {
		if (_onDec) {
			_onDec(buffer);
		}
		return;
	}
#if defined(ENABLE_OPENSSL)
	uint32_t offset = 0;
	while (offset < buffer->length()) {
		auto nwrite = BIO_write(_read_bio, buffer->data() + offset, buffer->length() - offset);
		if (nwrite > 0) {
			//部分或全部写入bio完毕
			offset += nwrite;
			flush();
			continue;
		}
		//nwrite <= 0,出现异常
		LogW(TAG, "ssl error");// SSLUtil::getLastError();
		shutdown();
		break;
	}
#endif
}

void SslFilter::onSend(shared_ptr<ByteBuffer> buffer) {
	if (!buffer->length()) {
		return;
	}
	if (!mSSL) {
		if (_onEnc) {
			_onEnc(buffer);
		}
		return;
	}

#if defined(ENABLE_OPENSSL)
	if (!mServerMode && !mSendHandshake) {
		mSendHandshake = true;
		SSL_do_handshake(mSSL.get());
	}
	_bufferOut.emplace_back(buffer);
	flush();
#endif
}

void SslFilter::flushWriteBio() {
#if defined(ENABLE_OPENSSL)
	int total = 0;
	int nread = 0;
	auto bufferBio = make_shared<ByteBuffer>();// _bufferPool.obtain();
	bufferBio->PrepareBuf(mBufSize);
	int buf_size = bufferBio->GetTailFreeSize() - 1;
	do {
		nread = BIO_read(_write_bio, bufferBio->data() + total, buf_size - total);
		if (nread > 0) {
			total += nread;
		}
	} while (nread > 0 && buf_size - total > 0);

	if (!total) {
		//未有数据
		return;
	}

	//触发此次回调
	bufferBio->data()[total] = '\0';
	bufferBio->WriteDirect(total);// setSize(total);
	if (_onEnc) {
		_onEnc(bufferBio);
	}

	if (nread > 0) {
		//还有剩余数据，读取剩余数据
		flushWriteBio();
	}
#endif 
}

void SslFilter::flushReadBio() {
#if defined(ENABLE_OPENSSL)
	int total = 0;
	int nread = 0;
	auto bufferBio = make_shared<ByteBuffer>();// _bufferPool.obtain();
	bufferBio->PrepareBuf(mBufSize);
	int buf_size = bufferBio->GetTailFreeSize() - 1;
	do {
		nread = SSL_read(mSSL.get(), bufferBio->data() + total, buf_size - total);
		if (nread > 0) {
			total += nread;
		}
	} while (nread > 0 && buf_size - total > 0);

	if (!total) {
		//未有数据
		return;
	}

	//触发此次回调
	bufferBio->data()[total] = '\0';
	bufferBio->WriteDirect(total);
	if (_onDec) {
		_onDec(bufferBio);
	}

	if (nread > 0) {
		//还有剩余数据，读取剩余数据
		flushReadBio();
	}
#endif 
}
void SslFilter::flush() {
#if defined(ENABLE_OPENSSL)
	flushReadBio();
	if (!SSL_is_init_finished(mSSL.get()) || _bufferOut.empty()) {
		//ssl未握手结束或没有需要发送的数据
		flushWriteBio();
		return;
	}

	//加密数据并发送
	while (!_bufferOut.empty()) {
		auto& front = _bufferOut.front();
		uint32_t offset = 0;
		while (offset < front->length()) {
			auto nwrite = SSL_write(mSSL.get(), front->data() + offset, front->length() - offset);
			if (nwrite > 0) {
				//部分或全部写入完毕
				offset += nwrite;
				flushWriteBio();
				continue;
			}
			//nwrite <= 0,出现异常
			break;
		}

		if (offset != front->length()) {
			//这个包未消费完毕，出现了异常,清空数据并断开ssl
			LogW(TAG, "ssl error:");// SSLUtil::getLastError();
			shutdown();
			break;
		}

		//这个包消费完毕，开始消费下一个包
		_bufferOut.pop_front();
	}
#endif 
}

bool SslFilter::setHost(const char* host) {
#ifdef SSL_ENABLE_SNI
	//return 0 != SSL_set_tlsext_host_name(mSSL.get(), host);
	//return 0 != wolfSSL_set_tlsext_host_name(mSSL.get(), host);
	LogW(TAG, "todo:%s",__func__);
#endif//SSL_ENABLE_SNI
	return false;
}


}
}
}

#endif
