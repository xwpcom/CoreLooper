#include "stdafx.h"
#include "tcplistener_windows.h"
#include "../../core/looper/handlerinternaldata.h"

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
	SockTool::CLOSE_SOCKET(mSock);
}

int TcpListener_Windows::StartListener(int port)
{
	if (!IsCreated())
	{
		/*
		�����������,����Ĵ������memory leak
		��򵥵İ취��ǿ��Ҫ��Create()Ȼ�����StartServer
		{
			auto obj = make_shared<TcpServer>();
			obj->StartServer(8081);
		}
		*/

		//Looper::CurrentLooper()->AddChild(shared_from_this());
		DW("%s fail,please call Create() and try again", __func__);
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
		DW("fail to StartServer(port=%d)", port);
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
			IoContext* context = new IoContext;//Stop()��GetQueuedCompletionStatus����acceptʧ��ʱdelete
			ret = PostAccept(context);
			if (ret == 0)
			{
				//ÿ���ɹ�Ͷ�ݵ�IoContext����Ҫ����һ��
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
			DW("lpfnAcceptEx failed with error code: %d/n", WSAGetLastError());

			return -1;
		}
	}

	return 0;
}

int TcpListener_Windows::DispatchIoContext(IoContext* context, DWORD bytes)
{
	auto objThis = shared_from_this();//ȷ����DispatchIoContextִ���ڼ䲻��ɾ��

	IoContextType type = context->mType;
	switch (type)
	{
	case IoContextType_Accept:
		bool postAgain = false;
		int ret = OnAccept(context->mSock);
		//���socketû�б��رգ����������
		if (ret == 0 && mSock != INVALID_SOCKET)
		{
			//����context
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
			//context�����Ŷ�TcpServer_Windows��һ������
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

	//�п���mSock�Ѿ����ر�
	
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
