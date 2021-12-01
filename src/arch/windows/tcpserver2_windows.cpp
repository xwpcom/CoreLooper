#include "stdafx.h"
#include "tcpserver2_windows.h"
#include "net/tcpserver.h"
#include "net/tcpclient.h"
#include "looper/looper.h"
#include "../../core/looper/handlerinternaldata.h"
#include "tcplistener_windows.h"

#ifdef _CONFIG_OPENSSL
#include "Util/SSLBox.h"
using namespace toolkit;
#endif


#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {

static const char* TAG = "TcpServer";

TcpServer2_Windows::TcpServer2_Windows()
{
	SetObjectName("TcpServer2");
}

TcpServer2_Windows::~TcpServer2_Windows()
{
}

void TcpServer2_Windows::OnAccept(Handler*,SOCKET s)
{
	auto client(CreateChannel());
	if (client)
	{
		AddChild(client);
		auto ret = client->OnConnect((long)s,nullptr);
		if (ret)
		{
			client->Destroy();
		}
	}
	else
	{
		SockTool::CLOSE_SOCKET(s);
	}
}

shared_ptr<Channel> TcpServer2_Windows::CreateChannel()
{
	auto client(make_shared<TcpClient>());
	return client;
}

#ifdef _CONFIG_OPENSSL
/*
中间证书检测地址：https://www.myssl.cn/tools/check-server-cert.html
通过此链接可以检测是否缺失中间证书；

https://www.myssl.cn/tools/check-server-cert.html
证书来源：	缺失证书
状态：	错误： 服务器缺少中间证书
*/

int TcpServer2_Windows::InitSSL(const string& filePath)
{
	Logger::Instance().add(std::make_shared<ConsoleChannel>());
	//Logger::Instance().add(std::make_shared<FileChannel>("FileChannel", ShellTool::GetAppPath() + "/log/"));
	Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

	//加载证书，证书包含公钥和私钥
	auto ok = SSL_Initor::Instance().loadCertificate(filePath.data());
	//信任某个自签名证书
	ok &= SSL_Initor::Instance().trustCertificate(filePath.data());
	//不忽略无效证书证书(例如自签名或过期证书)
	SSL_Initor::Instance().ignoreInvalidCertificate(false);
	return ok ? 0 : -1;
}
#endif

int TcpServer2_Windows::StartListenPort(int port)
{
	if (!IsCreated())
	{
		ASSERT(FALSE);
		return -1;
	}

	auto obj = make_shared<TcpListener_Windows>();
	AddChild(obj);
	auto ret=obj->StartListener(port);
	if (ret)
	{
		LogW(TAG, "fail open port %d", port);
		
		obj->Destroy();
		return -1;
	}

	obj->SignalAccept.connect(this, &TcpServer2_Windows::OnAccept);

	return 0;
}


}
}
}