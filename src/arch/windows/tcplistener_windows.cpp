﻿#include "stdafx.h"
#include "tcplistener_windows.h"
#include "../../core/looper/handlerinternaldata.h"
#include "profiler.h"

namespace Bear {
namespace Core
{
namespace Net {

static const char* TAG = "TcpListener";

TcpListener_Windows::TcpListener_Windows()
{
	mInternalData->SetActiveObject();
	SetObjectName("TcpListener");
}

TcpListener_Windows::~TcpListener_Windows()
{
	LogV(TAG, "%s,port=%d", __func__,mPort);

	SockTool::CLOSE_SOCKET(mSock);
}

int TcpListener_Windows::StartListener(int port)
{
	if (!IsCreated())
	{
		/*
		如果不加限制,下面的代码会有memory leak
		最简单的办法是强制要求Create()然后才能StartServer
		{
			auto obj = make_shared<TcpServer>();
			obj->StartServer(8081);
		}
		*/

		//Looper::CurrentLooper()->AddChild(shared_from_this());
		LogW(TAG,"%s fail,please call Create() and try again", __func__);
		ASSERT(FALSE);
		return -1;
	}

	if (mIocp != INVALID_HANDLE_VALUE)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	SockTool::ReuseAddr(sock);

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;// inet_addr("127.0.0.1");
	addr.sin_port = htons(port);//use port 0 for auto allocate port
	int ret = ::bind(sock, (LPSOCKADDR)&addr, sizeof(addr));
	if (ret)
	{
		LogW(TAG,"fail to StartServer(port=%d)", port);
		ASSERT(FALSE);

		SockTool::CAutoClose ac(&sock);
		return ret;
	}

	HANDLE hIocp = (HANDLE)Looper::CurrentLooper()->GetLooperHandle();
	ASSERT(hIocp);
	CreateIoCompletionPort((HANDLE)sock, hIocp, (ULONG_PTR)(IocpObject*)this, 0);

	ASSERT(mSock == INVALID_SOCKET);
	mIocp = hIocp;
	mSock = sock;

	//BindProcData(mPort, "port");

	if (port == 0)
	{
		struct sockaddr_in sa;
		socklen_t saLen = sizeof(sa);
		if (0 == getsockname(mSock, (struct sockaddr*)&sa, &saLen))
		{
			port = ntohs(sa.sin_port);
		}
	}

	mPort = port;

	ret = listen(mSock, 200);
	ASSERT(ret == 0);

	{
		//typedef int socklen_t;
		struct sockaddr_in ClientAddr;
		socklen_t ClientAddrLen = sizeof(ClientAddr);
		if (0 == getsockname(mSock, (struct sockaddr*)&ClientAddr, &ClientAddrLen))
		{
			//DV("port=%d", ntohs(ClientAddr.sin_port));
			int x = 0;
		}
	}

	if (mAcceptEx == NULL)
	{
		ASSERT(mGetAcceptExSockAddrs == NULL);

		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

		DWORD dwBytes = 0;
		ret = WSAIoctl(mSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &mAcceptEx,
			sizeof(mAcceptEx), &dwBytes, NULL, NULL);
		ASSERT(ret == 0);

		ret = WSAIoctl(mSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs,
			sizeof(GuidGetAcceptExSockAddrs), &mGetAcceptExSockAddrs, sizeof(mGetAcceptExSockAddrs),
			&dwBytes, NULL, NULL);

		for (int i = 0; i < 2; i++)
		{
			IoContext* context = new IoContext;//Stop()后返回accept失败时delete
			ret = PostAccept(context);
			if (ret == 0)
			{
				//每个成功投递的IoContext仅需要引用一次
				context->mBaseServer = dynamic_pointer_cast<TcpListener_Windows>(shared_from_this());
			}
			else
			{
				delete context;
				context = NULL;
				break;
			}
		}
	}

	return 0;
}

int TcpListener_Windows::PostAccept(IoContext* ioContext)
{
	ioContext->mByteBuffer.PrepareBuf(64);

	DWORD dwBytes = 0;
	ioContext->mType = IoContextType_Accept;
	ioContext->mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	BOOL ok = mAcceptEx(
		mSock,
		ioContext->mSock,
		ioContext->mByteBuffer.GetNewDataPointer(),
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&dwBytes,
		&(ioContext->mOV)
	);
	if (!ok)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			LogW(TAG,"lpfnAcceptEx failed with error code: %d/n", WSAGetLastError());

			return -1;
		}
	}

	return 0;
}

int TcpListener_Windows::DispatchIoContext(IoContext* context, DWORD bytes)
{
	auto objThis = shared_from_this();//确保在DispatchIoContext执行期间不被删除

	IoContextType type = context->mType;
	switch (type)
	{
	case IoContextType_Accept:
		bool postAgain = false;
		int ret = OnAccept(context->mSock);
#if defined _CONFIG_PROFILER
		if (Looper::CurrentLooper()->profilerEnabled())
		{
			auto obj = Looper::CurrentLooper()->profiler();
			obj->tcpAcceptCount++;
		}
#endif
		//如果socket没有被关闭，则继续侦听
		if (ret == 0 && mSock != INVALID_SOCKET)
		{
			//重用context
			context->mSock = INVALID_SOCKET;
			int ret = PostAccept(context);
			if (ret == 0)
			{
				postAgain = true;
			}
		}
		else
		{
			closesocket(context->mSock);
			context->mSock = INVALID_SOCKET;
		}

		if (postAgain)
		{
			//context保留着对TcpServer_Windows的一个引用
			//do nothing here
		}
		else
		{
			delete context;
			context = NULL;
		}

		break;
	}

	return 0;
}

int TcpListener_Windows::OnAccept(SOCKET s)
{
	//closesocket(s); return 0;

	int ret = -1;

	//有可能mSock已经被关闭
	
	ret = setsockopt(s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&mSock, sizeof(mSock));
	if (ret)
	{
		return -1;
	}
	
	SignalAccept(this, s);

	return 0;
}

void TcpListener_Windows::OnDestroy()
{
	__super::OnDestroy();

	StopListener();
}

void TcpListener_Windows::StopListener()
{
	if (mSock != INVALID_SOCKET)
	{
		shutdown(mSock, SD_BOTH);
		closesocket(mSock);
		mSock = INVALID_SOCKET;
	}
}

}
}
}
