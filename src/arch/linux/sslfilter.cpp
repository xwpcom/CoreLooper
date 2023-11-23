#include "stdafx.h"
#include "sslfilter.h"

#if defined _CONFIG_WOLFSSL
using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace std;

namespace Bear {
namespace Core {
namespace Net {

#define SSL_ENABLE_SNI
using namespace Bear::Core;
static const char* TAG = "SslFilter";
static bool mVerbose = true;//为true时打印调试信息

SslFilter::~SslFilter() {

	/*
	2022.06.27
	在windows下free _read_bio和_write_bio时app能正常结束
	在t21下面启用free _read_bio和_write_bio后app core dump,原因待查
	zlmediakit中对openssl没有启用free bio
	*/

	/*
	if (_read_bio)
	{
		LogV(TAG, "free _read_bio");
		BIO_free(_read_bio);
	}
	if (_write_bio)
	{
		LogV(TAG, "free _write_bio#begin");
		BIO_free(_write_bio);
	}
	//*/
}

SslFilter::SslFilter()
{
	//LogV(TAG, "%s(%p)",__func__,this);
}

void SslFilter::init()
{
	if (mVerbose)
	{
		LogV(TAG, "%s(%p)", __func__, this);
	}

	bool serverMode = false;

	_read_bio = BIO_new(BIO_s_mem());
	mServerMode = serverMode;
	{
		WOLFSSL_CTX* ctx = NULL;
		ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
		{
			wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0); //不验证

			/*
			auto ret = wolfSSL_CTX_load_verify_locations(ctx, "D:/iot.jjyip.com.p12"
				//, 0
				, R"(D:\os\wolfssl\certs\external)"
			);
			LogV(TAG, "wolfSSL_CTX_load_verify_locations ret=%d", ret);
			*/
		}

		auto ssl = SSL_new(ctx);
		mSSL = shared_ptr<SSL>(ssl, [](SSL* ptr) {
			wolfSSL_free(ptr);
			});

	}
	if (mSSL) {
		_write_bio = BIO_new(BIO_s_mem());
		SSL_set_bio(mSSL.get(), _read_bio, _write_bio);
		mServerMode ? SSL_set_accept_state(mSSL.get()) : SSL_set_connect_state(mSSL.get());
	}

	mSendHandshake = false;
	//mBufSize = buffSize;
}

void SslFilter::shutdown() {

	_bufferOut.clear();
	int ret = wolfSSL_shutdown(mSSL.get());
	if (ret != 1) {
		LogW(TAG, "SSL shutdown failed:");// << SSLUtil::getLastError();
	}
	else {
		flush();
	}

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

	uint32_t offset = 0;
	while (offset < buffer->length()) {
		auto nwrite = wolfSSL_BIO_write(_read_bio, buffer->data() + offset, buffer->length() - offset);
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

}

void SslFilter::onSend(shared_ptr<ByteBuffer> buffer) {

	if (mVerbose)
	{
		LogV(TAG, "%s",__func__);
	}

	if (!buffer->length()) {
		return;
	}
	if (!mSSL) {
		if (_onEnc) {
			_onEnc(buffer);
		}
		return;
	}

	if (!mServerMode && !mSendHandshake) {
		mSendHandshake = true;
		//SSL_do_handshake(mSSL.get());
		if (mVerbose)
		{
			LogV(TAG, "wolfSSL_connect");
		}
		wolfSSL_connect(mSSL.get());
	}
	_bufferOut.emplace_back(buffer);
	flush();
}

void SslFilter::flushWriteBio() {
	int total = 0;
	int nread = 0;
	auto bufferBio = make_shared<ByteBuffer>();// _bufferPool.obtain();
	bufferBio->PrepareBuf(mBufSize);
	int buf_size = bufferBio->GetTailFreeSize() - 1;
	do {
		nread = wolfSSL_BIO_read(_write_bio, bufferBio->data() + total, buf_size - total);
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

}

void SslFilter::flushReadBio() {
	int total = 0;
	int nread = 0;
	auto bufferBio = make_shared<ByteBuffer>();// _bufferPool.obtain();
	bufferBio->PrepareBuf(mBufSize);
	int buf_size = bufferBio->GetTailFreeSize() - 1;
	do {
		nread = wolfSSL_read(mSSL.get(), bufferBio->data() + total, buf_size - total);
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

}
void SslFilter::flush() {
	flushReadBio();
	if (!wolfSSL_is_init_finished(mSSL.get()) || _bufferOut.empty()) {
		//ssl未握手结束或没有需要发送的数据
		flushWriteBio();
		return;
	}

	//加密数据并发送
	while (!_bufferOut.empty()) {
		auto& front = _bufferOut.front();
		uint32_t offset = 0;
		while (offset < front->length()) {
			auto nwrite = wolfSSL_write(mSSL.get(), front->data() + offset, front->length() - offset);
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
}

bool SslFilter::setHost(const char* host) {
#ifdef SSL_ENABLE_SNI
	//return 0 != SSL_set_tlsext_host_name(mSSL.get(), host);
	//return 0 != wolfSSL_set_tlsext_host_name(mSSL.get(), host);
	LogW(TAG, "todo:%s",__func__);
#endif//SSL_ENABLE_SNI
	return false;
}

//为clientMode时连接成功后调用本接口,要进行handshake操作
void SslFilter::onConnect()
{
	if (mVerbose)
	{
		LogV(TAG, "%s(%p)", __func__, this);
	}

	if (!mSendHandshake)
	{
		auto ret = wolfSSL_connect(mSSL.get());
		if (mVerbose)
		{
			LogV(TAG, "wolfSSL_connect ret=%d", ret);
		}
		mSendHandshake = true;

		flush();
	}
}


}
}
}

string host2ip(const string& host)
{
	DWORD dwRetval;
	struct addrinfo* result = NULL;
	struct addrinfo* ptr = NULL;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//LogV(TAG,"getaddrinfo(%s)#begin", mDns);
	dwRetval = getaddrinfo(host.c_str(), nullptr, &hints, &result);//注意在断网情况下，此api可能阻塞60秒或更长时间
	if (dwRetval == 0)
	{
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			sockaddr_in* sa = (sockaddr_in*)ptr->ai_addr;
			//DT("name: %s ip:%s", ptr->ai_canonname,
			  //    inet_ntop(AF_INET, &sa->sin_addr.s_addr, ip, sizeof (ip)));  
			const char* ipThis = inet_ntoa(sa->sin_addr);
			//DT("Length of this sockaddr: %d,ip=[%s]", ptr->ai_addrlen,ip);
			//LogV(TAG,"Canonical name: %s,ipThis=%s", ptr->ai_canonname,ipThis);
			if (ipThis)
			{
				#ifdef _MSC_VER_DEBUG
				LogV(TAG, "[%s]=[%s]", host.c_str(), ipThis);
				#endif
				return ipThis;
				//strncpy(mIP, ipThis, sizeof(mIP) - 1);
				break;
			}
		}
	}

	return "";
}

//static const char* TAG = "wolfssl";
int testWolfSSL(const char* host, int port)
{
	printf("%s %s:%d\r\n", __FUNCTION__, host, port);
	int ret = wolfSSL_Init();
	printf("wolfSSL_Init=%d\r\n", ret);

	WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
	wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);//不加的话wireshark中能看到client向server发Fatal Alert - Unknown CA

	/*
	ret = wolfSSL_CTX_load_verify_locations(ctx, "D:/iot.jjyip.com.p12"
		//, 0
		, R"(D:\os\wolfssl\certs\external)"
	);
	LogV(TAG, "wolfSSL_CTX_load_verify_locations ret=%d", ret);
	*/

	int s = -1;
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//ip = "210.76.75.21";
		//port = 5443;
		//ip = host2ip("iot.jjyip.com");
		//ip = host2ip("163.com");
		//port = 443;

		struct sockaddr_in servAddr;
		servAddr.sin_addr.s_addr = inet_addr(host);
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(port);

		struct timeval tval;
		tval.tv_sec = 5;
		tval.tv_usec = 0;
		int ret = connect(s, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
		printf("connec ret = %d\r\n", ret);

	}

	//https://github.com/dktran65/MyWolfssl
	/*
	Note 2) wolfSSL takes a different approach to certificate verification than OpenSSL does.
	The default policy for the client is to verify the server,
	this means that if you don't load CAs to verify the server you'll get a connect error,
	no signer error to confirm failure (-188).
	If you want to mimic OpenSSL behavior of having SSL_connect succeed
	even if verifying the server failsand reducing security you can do this by calling :
	wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
	另一个解决办法是在wolfSSL_CTX_load_verify_locations用第三个参数加载root pem
	*/
	//wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);

	WOLFSSL* ssl = wolfSSL_new(ctx);
	ret = wolfSSL_set_fd(ssl, s);
	//wolfSSL_set_tlsext_host_name(ssl, "163.com");

	printf("wolfSSL_set_fd ret = %d\r\n", ret);
	char req[256] = { 0 };
	snprintf(req, sizeof(req),
			 "GET / HTTP/1.1\r\n"
			 "\r\n"
	);

	//Sleep(2000);
	ret = wolfSSL_connect(ssl);
	printf("wolfSSL_connect ret=%d\r\n", ret);
	ret = wolfSSL_write(ssl, req, strlen(req));
	printf("wolfSSL_write ret= %d\r\n", ret);

	char ack[8 * 1024] = { 0 };
	ret = wolfSSL_read(ssl, ack, sizeof(ack) - 1);
	printf("wolfSSL_read ret = %d\r\n", ret);
	if (ret > 0)
	{
		ack[ret] = 0;
		printf("recv:%s\r\n", ack);
	}

	//auto _read_bio = BIO_new(BIO_s_mem());

	return 0;
}


#endif
