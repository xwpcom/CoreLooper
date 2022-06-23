#include "stdafx.h"
#include "SSLBox.h"
/*
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 */
#include <string.h>
#include "util.h"
#include "onceToken.h"
#include "SSLUtil.h"

using namespace Bear::Core;
static const char* TAG = "sslBox";
static bool mVerbose = true;//为true时打印调试信息

#if defined(ENABLE_OPENSSL)
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/bio.h>
#include <openssl/ossl_typ.h>
#endif

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
//openssl版本是否支持sni
#define SSL_ENABLE_SNI
#endif

namespace toolkit {

SSL_Box::~SSL_Box() {}

SSL_Box::SSL_Box(bool serverMode, bool enable, int buffSize) {
#if defined(ENABLE_OPENSSL)
    _read_bio = BIO_new(BIO_s_mem());
    _serverMode = serverMode;
    if(enable){
        _ssl = SSL_Initor::Instance().makeSSL(serverMode);
    }
    if(_ssl){
		_write_bio = BIO_new(BIO_s_mem());
		SSL_set_bio(_ssl.get(), _read_bio, _write_bio);
		_serverMode ? SSL_set_accept_state(_ssl.get()) : SSL_set_connect_state(_ssl.get());
	} else {
		WarnL << "ssl disabled!";
	}
	_sendHandshake = false;
	_buffSize = buffSize;
#endif 
}

void SSL_Box::shutdown() {
#if defined(ENABLE_OPENSSL)
	_bufferOut.clear();
	int ret = SSL_shutdown(_ssl.get());
	if (ret != 1) {
		ErrorL << "SSL shutdown failed:" << SSLUtil::getLastError();
	} else {
		flush();
	}
#endif 
}

void SSL_Box::onRecv(const Buffer::Ptr &buffer) {
	if(!buffer->size()){
		return;
	}
    if (!_ssl) {
		if (_onDec) {
			_onDec(buffer);
		}
		return;
	}
#if defined(ENABLE_OPENSSL)
	uint32_t offset = 0;
	while(offset < buffer->size()){
		auto nwrite = BIO_write(_read_bio, buffer->data() + offset, buffer->size() - offset);
		if (nwrite > 0) {
			//部分或全部写入bio完毕
			offset += nwrite;
			flush();
			continue;
		}
		//nwrite <= 0,出现异常
		ErrorL << "ssl error:" << SSLUtil::getLastError();
		shutdown();
		break;
	}
#endif
}

void SSL_Box::onSend(const Buffer::Ptr &buffer) {
	if(!buffer->size()){
		return;
	}
	if (!_ssl) {
		if (_onEnc) {
			_onEnc(buffer);
		}
		return;
	}

#if defined(ENABLE_OPENSSL)
    if (!_serverMode && !_sendHandshake) {
		_sendHandshake = true;
		SSL_do_handshake(_ssl.get());
	}
	_bufferOut.emplace_back(buffer);
	flush();
#endif
}

void SSL_Box::flushWriteBio() {
#if defined(ENABLE_OPENSSL)
    int total = 0;
	int nread = 0;
	auto bufferBio = _bufferPool.obtain();
	bufferBio->setCapacity(_buffSize);
	int buf_size = bufferBio->getCapacity() - 1;
	do{
		nread = BIO_read(_write_bio, bufferBio->data() + total, buf_size - total);
		if(nread > 0){
			total += nread;
		}
	}while(nread > 0 && buf_size - total  > 0);

	if(!total){
		//未有数据
		return;
	}

	//触发此次回调
	bufferBio->data()[total] = '\0';
	bufferBio->setSize(total);
	if(_onEnc){
		_onEnc(bufferBio);
	}

	if(nread > 0){
		//还有剩余数据，读取剩余数据
		flushWriteBio();
	}
#endif 
}

void SSL_Box::flushReadBio() {
#if defined(ENABLE_OPENSSL)
    int total = 0;
	int nread = 0;
	auto bufferBio = _bufferPool.obtain();
	bufferBio->setCapacity(_buffSize);
	int buf_size = bufferBio->getCapacity() - 1;
	do{
		nread = SSL_read(_ssl.get(), bufferBio->data() + total, buf_size - total);
		if(nread > 0){
			total += nread;
		}
	}while(nread > 0 && buf_size - total  > 0);

	if(!total){
		//未有数据
		return;
	}

	//触发此次回调
	bufferBio->data()[total] = '\0';
	bufferBio->setSize(total);
	if(_onDec){
		_onDec(bufferBio);
	}

	if(nread > 0){
		//还有剩余数据，读取剩余数据
		flushReadBio();
	}
#endif 
}
void SSL_Box::flush() {
#if defined(ENABLE_OPENSSL)
    flushReadBio();
	if (!SSL_is_init_finished(_ssl.get()) || _bufferOut.empty()) {
		//ssl未握手结束或没有需要发送的数据
		flushWriteBio();
        return;
	}

	//加密数据并发送
	while (!_bufferOut.empty()){
		auto &front = _bufferOut.front();
		uint32_t offset = 0;
		while(offset < front->size()){
			auto nwrite = SSL_write(_ssl.get(), front->data() + offset, front->size() - offset);
			if (nwrite > 0) {
				//部分或全部写入完毕
				offset += nwrite;
				flushWriteBio();
				continue;
			}
			//nwrite <= 0,出现异常
			break;
		}

		if(offset != front->size()){
			//这个包未消费完毕，出现了异常,清空数据并断开ssl
			ErrorL << "ssl error:" << SSLUtil::getLastError() ;
			shutdown();
			break;
		}

		//这个包消费完毕，开始消费下一个包
		_bufferOut.pop_front();
	}
#endif 
}

bool SSL_Box::setHost(const char *host) {
#ifdef SSL_ENABLE_SNI
	return 0 != SSL_set_tlsext_host_name(_ssl.get(), host);
#endif//SSL_ENABLE_SNI
	return false;
}


}
