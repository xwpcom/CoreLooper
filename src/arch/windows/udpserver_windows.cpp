#include "stdafx.h"
#include "udpserver_windows.h"
#include "UdpIoContext.h"
#include "udpclient_windows.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {
UdpServer_Windows::UdpServer_Windows()
{
	SetObjectName("UdpServer");

	mIocp = INVALID_HANDLE_VALUE;
	mSock = INVALID_SOCKET;
	mPort = 0;

	//DV("%s", __func__);
}

UdpServer_Windows::~UdpServer_Windows()
{
	SockTool::CLOSE_SOCKET(mSock);

	//DV("%s", __func__);
}

int UdpServer_Windows::StartServer(int port)
{
	auto sock = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	addr.sin_port = htons(port);//use port 0 for auto allocate port
	int ret = ::bind(sock, (LPSOCKADDR)&addr, sizeof(addr));
	if (ret)
	{
		DW("fail to StartServer(port=%d)", port);
		ASSERT(FALSE);

		SockTool::CAutoClose ac(&sock);
		return ret;
	}

	HANDLE hIocp = (HANDLE)Looper::CurrentLooper()->GetLooperHandle();
	ASSERT(hIocp);
	auto handle = CreateIoCompletionPort((HANDLE)sock, hIocp, (ULONG_PTR)(IocpObject*)this, 0);
	ASSERT(handle == hIocp);

	ASSERT(mSock == INVALID_SOCKET);
	mIocp = hIocp;
	mSock = sock;

	//BindProcData(mPort, "port");

	if (port == 0)
	{
		struct sockaddr_in sa;
		socklen_t saLen = sizeof(sa);
		if (0 == getsockname(mSock, (struct sockaddr *)&sa, &saLen))
		{
			port = ntohs(sa.sin_port);
		}
	}

	mPort = port;

	for (int i = 0; i < 2; i++)
	{
		auto context = make_shared<UdpIoContext>();
		context->mSelfRef = context;
		context->mBaseServer = dynamic_pointer_cast<UdpServer_Windows>(shared_from_this());
		context->mSock = mSock;
		context->mType = IoContextType_Recv;
		context->mByteBuffer.PrepareBuf(64 * 1024);
		context->PostRecv();
	}

	return 0;
}

int UdpServer_Windows::DispatchIoContext(IoContext *context2, DWORD bytes)
{
	UdpIoContext *context = (UdpIoContext *)context2;

	auto objThis = shared_from_this();//确保在DispatchIoContext执行期间不被删除

	ASSERT(context->mBusying);
	context->mBusying = false;
	IoContextType type = context->mType;
	switch (type)
	{
	case IoContextType_Recv:
	{
		if (bytes > 0)
		{
			OnRecv(context->mByteBuffer.GetDataPointer(), bytes, context->mSockAddr);
		}

		if (mSock != INVALID_SOCKET)
		{
			context->PostRecv();
		}
		else
		{
			PostDispose(context->mBaseServer);
			context->mSelfRef = nullptr;
		}
		break;
	}
	default:
	{
		ASSERT(FALSE);
		break;
	}
	}

	return 0;
}

void UdpServer_Windows::Stop()
{
	if (mSock != INVALID_SOCKET)
	{
		shutdown(mSock, SD_BOTH);
		closesocket(mSock);
		mSock = INVALID_SOCKET;
	}
}

LRESULT UdpServer_Windows::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		Stop();
		break;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

void UdpServer_Windows::OnRecv(LPBYTE data, int dataBytes, const sockaddr& addr)
{
	static int idx = -1;
	++idx;

	string text((char*)data, dataBytes);
	DW("recv[%04d]=[%s]", idx, text.c_str());

	auto client = make_shared<UdpClient_Windows>();
	int x = 0;
}
}
}
}