#include "stdafx.h"
#include "net/channel.h"
#include "looper/Loop.h"
#include "libhttp/libhttp.inl"
#include "demohttps.h"
#include "net/tcpclient.h"
#include "tlsproxy.h"

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time       time 
#define mbedtls_time_t     time_t
#define mbedtls_fprintf    fprintf
#define DV     printf
#endif

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include <string.h>

#define SERVER_PORT "443"
#define SERVER_NAME "open.ys7.com" //"83.136.184.118"

#define DEBUG_LEVEL 1

static void my_debug(void *ctx, int level,
	const char *file, int line,
	const char *str)
{
	((void)level);

	mbedtls_fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
	fflush((FILE *)ctx);
}

int mbedtls_net_sendEx(void *ctx, const unsigned char *buf, size_t len)
{
	return mbedtls_net_send(ctx, buf, len);
}

int mbedtls_net_recvEx(void *ctx, unsigned char *buf, size_t len)
{
	return mbedtls_net_recv(ctx, buf, len);
}

class MbedTls
{
public:
	MbedTls()
	{
		//Initialize the RNG and the session data
		mbedtls_net_init(&server_fd);
		mbedtls_ssl_init(&ssl);
		mbedtls_ssl_config_init(&conf);
		//mbedtls_x509_crt_init(&cacert);
		mbedtls_ctr_drbg_init(&ctr_drbg);

		mbedtls_entropy_init(&entropy);
		const char *pers = "jjy";
		auto ret= mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
			(const unsigned char *)pers,
			strlen(pers));
		ASSERT(ret == 0);
	}

	int Connect(string host,string port)
	{
		auto ret = mbedtls_net_connect(&server_fd, host.c_str(), port.c_str(), MBEDTLS_NET_PROTO_TCP);
		if (ret == 0)
		{
			mbedtls_net_set_nonblock(&server_fd);
		}
		return ret;
	}

	int Setup()
	{
		auto ret = mbedtls_ssl_config_defaults(&conf,
			MBEDTLS_SSL_IS_CLIENT,
			MBEDTLS_SSL_TRANSPORT_STREAM,
			MBEDTLS_SSL_PRESET_DEFAULT);
		if (ret)
		{
			DV(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
			return -1;
		}

		DV(" ok\n");

		/* OPTIONAL is not optimal for security,
		* but makes interop easier in this simplified example */
		mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
		//mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
		mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
		mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

		if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
		{
			DV(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
			return -1;
		}

		if ((ret = mbedtls_ssl_set_hostname(&ssl, SERVER_NAME)) != 0)
		{
			DV(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
			return -1;
		}

		mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_sendEx, mbedtls_net_recvEx, NULL);
		mbedtls_ssl_conf_read_timeout(&conf, 15*1000);
		/*
		* 4. Handshake
		*/
		DV("  . Performing the SSL/TLS handshake...");


		while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			{
				DV(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
				return -1;
			}
		}

		return 0;
	}

	int Write(LPBYTE buf,int len)
	{
		return mbedtls_ssl_write(&ssl, buf, len);
	}

	int Read(LPBYTE buf, int len)
	{
		return mbedtls_ssl_read(&ssl, buf, len);
	}

	void Close()
	{
		mbedtls_ssl_close_notify(&ssl);
		mbedtls_net_free(&server_fd);

		mbedtls_ssl_free(&ssl);
		mbedtls_ssl_config_free(&conf);
		mbedtls_ctr_drbg_free(&ctr_drbg);
		mbedtls_entropy_free(&entropy);
	}
protected:
	mbedtls_net_context server_fd;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;

};

//参考mbedtls/programs/ssl/ssl_client1.c
int TestSsl()
{
	int ret, len;
	//uint32_t flags;
	unsigned char buf[1024*2];

	MbedTls tls;
	
	//todo:用TcpClient连接上socket之后，直接提供给tls绑定server_fd.fd = sock;
	ret = tls.Connect(SERVER_NAME,SERVER_PORT);

	tls.Setup();
	
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"
	if (1)
	{
		len = sprintf((char *)buf, "%s", GET_REQUEST);
	}
	else
	{
		string appKey = "3020f008059c4ddda8e845f34df2a2c8";
		string secret = "3f5e68bd464b36746c35111d451e490d";

		string body = StringTool::Format(
			"appKey=%s&appSecret=%s"
			, appKey.c_str()
			, secret.c_str()
		);

		string  req = StringTool::Format(
			"POST /api/lapp/token/get HTTP/1.1\r\n"
			"Host: open.ys7.com\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"Content-Length: %d\r\n"
			"\r\n"
			"%s"
			,body.length()
			,body.c_str()
		);

		string filePath = "d:/http.req.txt";
		File::Dump(req, filePath.c_str());

		len = sprintf((char *)buf, "%s",req.c_str());
	}

	while ((ret = tls.Write(buf, len)) <= 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			DV(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
			return -1;
		}
	}

	len = ret;
	DV(" %d bytes written[%s]", len, (char *)buf);

	do
	{
		len = sizeof(buf) - 1;
		memset(buf, 0, sizeof(buf));
		ret = tls.Read(buf, len);

		if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;

		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
			break;

		if (ret < 0)
		{
			DV("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
			break;
		}

		if (ret == 0)
		{
			DV("\n\nEOF\n\n");
			break;
		}

		len = ret;

		{
			static int idx = -1;
			++idx;

			string filePath = StringTool::Format("d:/http.ack_%04d.txt", idx);
			File::Dump(buf, len, filePath);
		}

		DV(" %d bytes read[%s]", len, (char *)buf);
		break;
	} while (1);

	tls.Close();

	return ret;
}


using namespace Bear::Core;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Http;
namespace Bear {
namespace Core {
namespace Net {
namespace Https {

DemoHttps::DemoHttps()
{
	SetObjectName("DemoHttps");
	DV("%s,this=0x%08x", __func__, this);

	mDumpFile.Open(ShellTool::GetAppPath() + "/http.recv.bin");
}

DemoHttps::~DemoHttps()
{
	DV("%s,this=0x%08x", __func__, this);
}

LRESULT DemoHttps::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		PostDispose(mChannel);
		break;
	}
	}

	auto ret = Handler::OnMessage(msg, wp, lp);
	return ret;
}

void DemoHttps::OnClose(Channel*)
{
	DW("%s", __func__);

	if (mChannel)
	{
		mChannel->Destroy();
		mChannel = nullptr;
	}
	
	Destroy();
}

void DemoHttps::OnSend(Channel*)
{
	int x = 0;
}


void DemoHttps::OnConnect(Channel *endPoint, long error, ByteBuffer *pBox, Bundle* extraInfo)
{
	if (error)
	{
		DW("%s,connect fail", __func__);

		PostDispose(mChannel);
		Destroy();
	}
	else
	{
		DV("%s,connect ok", __func__);

		mInbox.PrepareBuf(4 * 1024);
		mOutbox.PrepareBuf(4 * 1024);

		mProxy = make_shared<Crypt::TlsProxy>();
		//auto obj = dynamic_pointer_cast<TcpClient>(endPoint->shared_from_this());
		auto s = (SOCKET)endPoint->GetHandle();
		mProxy->Init(s, "ys7.com", 443, "", "");
		SendRequest();
	}
}

void DemoHttps::OnReceive(Channel*)
{
	while (mChannel)
	{
		mInbox.MoveToHead();
		int bytes = mInbox.GetTailFreeSize();
		if (bytes <= 1)
		{
			break;
		}

		int ret = mChannel->Receive(mInbox.GetNewDataPointer(), bytes - 1);
		if (ret <= 0)
		{
			break;
		}

		mInbox.WriteDirect(ret);
		mInbox.MakeSureEndWithNull();

		ParseInbox();
	}
}

//解析出inbox中所有的http ack,可能含多个ack
void DemoHttps::ParseInbox()
{
	ASSERT(mInbox.IsEndWithNull());

	while (!mInbox.IsEmpty())
	{
		switch (mHttpAckInfo.mHttpAckStatus)
		{
		case eHttpAckStatus_WaitHeader:
		{
			const char *ps = (const char*)mInbox.GetDataPointer();
			const char *key = "\r\n\r\n";
			const char *pEnd = strstr(ps, key);
			if (!pEnd)
			{
				return;
			}

			string  header(ps, pEnd - ps);

			{
				static int idx = -1;
				DV("S=>C [%04d] %s", ++idx, header.c_str());
			}

			mHttpAckInfo.mContentLength = HttpTool::GetInt(header, "Content-Length");
			if (mHttpAckInfo.mContentLength > 0)
			{
				SwitchStatus(eHttpAckStatus_ReceivingBody);
			}
			else
			{
				SwitchStatus(eHttpAckStatus_Done);
			}

			int eat = (int)(pEnd + strlen(key) - ps);
			mInbox.Eat(eat);
			break;
		}
		case eHttpAckStatus_ReceivingBody:
		{
			ASSERT(mHttpAckInfo.mContentRecvBytes <= mHttpAckInfo.mContentLength);

			int maxEatBytes = mHttpAckInfo.mContentLength - mHttpAckInfo.mContentRecvBytes;
			int bytes = mInbox.GetActualDataLength();
			int len = MIN(maxEatBytes, bytes);
			mHttpAckInfo.mContentRecvBytes += len;
			OnRecvHttpAckBody(mInbox.GetDataPointer(), len);
			mInbox.Eat(len);
			mInbox.MoveToHead();

			if (mHttpAckInfo.mContentRecvBytes == mHttpAckInfo.mContentLength)
			{
				mHttpAckInfo.Reset();
			}

			break;
		}
		case eHttpAckStatus_Done:
		{
			break;
		}
		default:
		{
			ASSERT(FALSE);
			break;
		}
		}
	}
}

void DemoHttps::OnRecvHttpAckBody(LPVOID data, int dataLen)
{
	mDumpFile.Write(data, dataLen);
}

void DemoHttps::SwitchStatus(DemoHttps::eHttpAckStatus status)
{
	mHttpAckInfo.mHttpAckStatus = status;
	switch (status)
	{
	case eHttpAckStatus_Done:
	{
		mHttpAckInfo.Reset();
		break;
	}
	}
}

void DemoHttps::SendRequest()
{
	ASSERT(mOutbox.IsEmpty());

	string appKey = "3020f008059c4ddda8e845f34df2a2c8";
	string secret = "3f5e68bd464b36746c35111d451e490d";

	string  req = StringTool::Format(
		"POST /api/lapp/token/get  HTTP/1.1\r\n"
		"Host: open.ys7.com\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"\r\n"
		"appKey=%s&appSecret=%s\r\n"
		, appKey.c_str()
		,secret.c_str()
	);

	int len = (int)req.length();
	mOutbox.Write((LPVOID)req.c_str(), len);
	CheckSend();
}

void DemoHttps::CheckSend()
{
	if (!mChannel)
	{
		return;
	}
}

void DemoHttps::OnTimer(long timerId)
{
	if(timerId == mTimerDelayExit)
	{
		KillTimer(mTimerDelayExit);

		Looper::GetMainLooper()->PostQuitMessage();
		return;
	}

	__super::OnTimer(timerId);
}

void DemoHttps::OnCreate()
{
	__super::OnCreate();

	TestSsl();

	if (1)
	{
		auto obj = make_shared<TcpClient>();
		obj->SignalOnConnect.connect(this, &DemoHttps::OnConnect);

		Bundle info;
		info.Set("address"
			, "183.136.184.118"
			//, "open.ys7.com"
		);
		info.Set("port", 443);
		obj->Connect(info);

		mChannel = obj;
	}

	SetTimer(mTimerDelayExit,20*1000);
}

}
}
}
}