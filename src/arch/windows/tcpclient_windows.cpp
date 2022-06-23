#include "stdafx.h"
#include "tcpclient_windows.h"
#include "net/tcpclient.h"
#include "net/tcpserver.h"
#include "looper/looper.h"
#include "net/dnslooper.h"
#include "../../core/looper/handlerinternaldata.h"
#include "../../core/looper/message.inl"
#include <string>
#include <sstream>
#ifdef _MSC_VER
#include <mstcpip.h>
#endif

#include <WinSock2.h>  
#include <MSWSock.h>  
#include <Windows.h>  
#include <process.h>  

#ifdef _CONFIG_OPENSSL
#include "Util/SSLBox.h"
#endif

#ifdef _MSC_VER_DEBUG
	#define new DEBUG_NEW
#endif

using namespace std;

namespace Bear {
namespace Core
{
namespace Net {
enum
{
	BM_DNS_ACK,
};

static const char* TAG = "TcpClient";

TcpClient_Windows::TcpClient_Windows()
{
	mInternalData->SetActiveObject();

	SetObjectName("TcpClient_Windows");
	SockTool::InitSockTool();

	//DV("%s,this=0x%x", __func__, this);
	mIocp = (HANDLE)Looper::CurrentLooper()->GetLooperHandle();
}

TcpClient_Windows::~TcpClient_Windows()
{
	//DV("%s,this=0x%x", __func__, this);
	ASSERT(mSock == INVALID_SOCKET);

	//ASSERT(IsMyselfThread());//确保在iocp线程中析构(是为了能直接调用TcpServer接口)
	//由于TcpClient_Windows.mBaseServer是强引用，所以运行到此时能保证TcpServer和Looper都存在并正常运行
}

int TcpClient_Windows::OnRecv(IoContext *context, DWORD bytes)
{
	ASSERT(context == &this->mIoContextRecv);

	context->mBusying = false;
	bool repost = false;
	auto inbox = GetRawInbox();
	ASSERT(inbox);

	if (bytes > 0)
	{
		UpdateRecvTick();

		int ret = inbox->Write((LPBYTE)context->mByteBuffer.GetDataPointer(), bytes);
		if (ret != bytes)
		{
			LogW(TAG,"fail write,ret=%d,bytes=%d", ret, bytes);
			ASSERT(FALSE);//todo
		}
	}
	else if (bytes == 0)
	{
		Close();
	}

	if (!mReceiveBusying)
	{
		/*避免重入问题 */
		mReceiveBusying = true;
		OnReceive();
		mReceiveBusying = false;
	}

	if (bytes > 0 && !context->mBusying)
	{
		inbox->MoveToHead();
		auto ret = context->PostRecv();
		if (ret)
		{
			Close();
		}
	}
	return 0;
}

int TcpClient_Windows::OnSendDone(IoContext *context, DWORD bytes)
{
	context->mBusying = false;
	context->mByteBuffer.clear();

	UpdateSendTick();

	if (mSock == INVALID_SOCKET)
	{
		context->mSock = INVALID_SOCKET;
		PostDispose(context->mBaseClient);
		return 0;
	}
	else
	{
		bool full = (mOutbox.GetFreeSize() == 0);
		if (!full)
		{
			OnSend();
		}

		if (mMarkEndOfSend && mOutbox.IsEmpty())
		{
			shutdown(mSock, SD_SEND);
		}

		if (!mOutbox.IsEmpty() && !mIoContextSend.mBusying)
		{
			int ret = SendOutBox();
			if (ret)
			{
				return -1;
			}
		}
	}

	return 0;
}

int TcpClient_Windows::DispatchIoContext(IoContext *context, DWORD bytes)
{
	auto objThis = shared_from_this();//确保在DispatchIoContext执行期间不被删除

	//shared_ptr<TcpClient_Windows> ptr = dynamic_pointer_cast<TcpClient_Windows>(shared_from_this());
	//2016.03.22,现在采用PostDispose来保证不会删除调用栈上的对象
	//所以不再需要在这里用shared_ptr保护TcpClient_Windows

	switch (context->mType)
	{
	case IoContextType_Recv:
	{
		int ret = OnRecv(context, bytes);
		break;
	}
	case IoContextType_Send:
	{
		OnSendDone(context, bytes);
		break;
	}
	case IoContextType_Connect:
	{
		mIoContextConnect.mBusying = false;
		mIoContextConnect.mBaseClient = nullptr;
		OnConnectAck();
		break;
	}
	default:
	{
		ASSERT(FALSE);
		break;
	}
	}

	if (mSingalClosePending)
	{
		OnClose();
	}

	return 0;
}

void TcpClient_Windows::OnReceive()
{
	bool fireEvent = true;

#ifdef _CONFIG_OPENSSL
	if (mTlsInfo)
	{
		fireEvent = false;
		if (mTlsInfo->mSslBox && !mTlsInfo->mInboxSSL.empty())
		{
			char* data = (char*)mTlsInfo->mInboxSSL.data();
			int bytes = mTlsInfo->mInboxSSL.length();
			mTlsInfo->mInBuffer->assign((char*)data, bytes);
			mTlsInfo->mInboxSSL.clear();
			//DV("ssl.onRecv,bytes=%d", bytes);
			mTlsInfo->mSslBox->onRecv(mTlsInfo->mInBuffer);
		}
	}
#endif

	if (fireEvent)
	{
		SignalOnReceive(this);
	}
}

//当可写时会调用本接口
void TcpClient_Windows::OnSend()
{
	SignalOnSend(this);
}

void TcpClient_Windows::OnClose()
{
	if (mSignalCloseHasFired)
	{
		return;
	}

	bool busying = (mIoContextConnect.mBusying || mIoContextRecv.mBusying || mIoContextSend.mBusying);
	if (busying)
	{
		mSingalClosePending = true;
		return;
	}

	mSingalClosePending = false;
	mSignalCloseHasFired = true;
	SignalOnClose(this);
}

//返回接收到的字节数
int TcpClient_Windows::Receive(LPVOID buf, int bufLen)
{
	ASSERT(buf);

	int bytes = MIN(mInbox.GetActualDataLength(), bufLen);
	if (bytes > 0)
	{
		memcpy(buf, mInbox.GetDataPointer(), bytes);
		mInbox.Eat(bytes);

		if (!mIoContextRecv.mBusying)
		{
			int freeBytes = mInbox.GetTailFreeSize();
			if (freeBytes == 0)
			{
				mInbox.MoveToHead();
			}

			freeBytes = mInbox.GetTailFreeSize();
			ASSERT(freeBytes > 0);
			if (mIoContextRecv.PostRecv(freeBytes))
			{
				Close();
			}
		}
		return bytes;
	}
	else if (mSock == INVALID_SOCKET)
	{
		return 0;
	}

	return -1;
}

//返回成功提交的字节数
int TcpClient_Windows::Send(LPVOID data, int dataLen)
{
	ASSERT(IsMyselfThread());

#ifdef _CONFIG_OPENSSL
	if (mTlsInfo)
	{
		auto outBuffer = make_shared<BufferRaw>();
		outBuffer->assign((char*)data, dataLen);
		if (mTlsInfo->mSslBox)
		{
			mTlsInfo->mSslBox->onSend(outBuffer);
		}

		return dataLen;
	}
#endif

	int freeSize = mOutbox.GetFreeSize();
	int bytes = MIN(dataLen, freeSize);
	int ret = mOutbox.Write(data, bytes);

	CheckSend();
	return ret>=0?ret:0;
}

void TcpClient_Windows::CheckSend()
{
	if (!mIoContextSend.mBusying)
	{
		int ack = SendOutBox();
		if (ack)
		{
			Close();
		}
	}
}

int TcpClient_Windows::SendOutBox()
{
	ASSERT(!mIoContextSend.mBusying);
	ASSERT(mIoContextSend.mByteBuffer.IsEmpty());
	if (mOutbox.IsEmpty())
	{
		return -1;
	}

	int ret = mIoContextSend.mByteBuffer.Write(mOutbox.GetDataPointer(), mOutbox.GetActualDataLength());
	if (ret > 0)
	{
		int ack = mIoContextSend.PostSend();
		if (ack)
		{
			Close();
			return -1;
		}

		mOutbox.Eat(ret);
		return 0;
	}

	return -1;
}

int TcpClient_Windows::ConnectHelper(string ip)
{
	//ASSERT(SockTool::IsValidIP(ip));
	ASSERT(mSock == INVALID_SOCKET);

	int port = mBundle.GetInt("port");

	mSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	HANDLE handle = CreateIoCompletionPort((HANDLE)mSock, mIocp, (ULONG_PTR)this, 0);
	ASSERT(handle == mIocp);

	{
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = 0;
		int ret = ::bind(mSock, (sockaddr*)&sin, sizeof(sin));
		//DT("mSock=%d,bind ret=%d", mSock,ret);
	}

	{
		DWORD ulBytesReturn = 0;
		struct tcp_keepalive stKeepIn = { 0 }, stKeepOut = { 0 };
		stKeepIn.keepalivetime = 1000 * 60*5;		// 超过20秒没接收数据就发送探测包
		stKeepIn.keepaliveinterval = 1000 * 20;	// 探测包ack超时后每隔2秒发送一次探测包
		stKeepIn.onoff = 1;						// 启用KEEPALIVE
		int ret = WSAIoctl(mSock, SIO_KEEPALIVE_VALS, (LPVOID)&stKeepIn, sizeof(tcp_keepalive), (LPVOID)&stKeepOut,
			sizeof(tcp_keepalive), &ulBytesReturn, NULL, NULL);
		if (ret == SOCKET_ERROR)
		{
			LogW(TAG,"CTCPNetwork::AddSession - WSAIoctl failed. errno(%d)", WSAGetLastError());
		}
	}

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ip.c_str());
	sin.sin_port = htons(port);

	mIoContextConnect.mType = IoContextType_Connect;
	mIoContextConnect.mBaseClient = dynamic_pointer_cast<TcpClient>(shared_from_this());

	//采用ConnectEx之前要绑定本地sock,否则调用会失败
	//ConnectEx默认超时为20秒
	if (!m_lpfnConnectEx)
	{
		GUID guidConnectEx = WSAID_CONNECTEX;
		DWORD dwBytes = 0;
		int ret = WSAIoctl(mSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidConnectEx, sizeof(guidConnectEx),
			&m_lpfnConnectEx, sizeof(m_lpfnConnectEx), &dwBytes, NULL, NULL);
		ASSERT(ret == 0);
	}

	BOOL ok = m_lpfnConnectEx(mSock, (sockaddr*)&sin, sizeof(sockaddr), NULL, 0, 0, &(mIoContextConnect.mOV));
	if (!ok)
	{
		int error = WSAGetLastError();
		if (error == ERROR_IO_PENDING)
		{
			mIoContextConnect.mBusying = true;
			//DT("ERROR_IO_PENDING");
			ok = true;
		}
		else
		{
			LogW(TAG,"error = %d", error);
		}
	}

	return 0;
}

int TcpClient_Windows::Connect(Bundle& info)
{
	if (mSock != INVALID_SOCKET)
	{
		ASSERT(FALSE);
		return -1;
	}

	//BindProcData(mAddress, "address");

	mBundle = info;
	string addr = info.GetString("address");
	mAddress = addr.c_str();
	if (SockTool::IsValidIP(addr.c_str()))
	{
		return ConnectHelper(addr);
	}

	auto mainLooper = Looper::GetMainLooper();
	ASSERT(mainLooper);

	if (mainLooper)
	{
		auto dnsLooper = _MObject(DnsLooper, "DnsLooper");
		if (!dnsLooper)
		{
			auto looper = make_shared<DnsLooper>();
			mainLooper->AddChild(looper);
			looper->Start();

			dnsLooper = _MObject(DnsLooper,"DnsLooper");
		}

		if (dnsLooper)
		{
			dnsLooper->AddRequest(addr, shared_from_this(), BM_DNS_ACK);
		}
	}

	return 0;
}

void TcpClient_Windows::Close()
{
	ASSERT(IsMyselfThread());

	bool needFireEvent = false;
	if (mSock != INVALID_SOCKET)
	{
		shutdown(mSock, SD_BOTH);
		closesocket(mSock);
		mSock = INVALID_SOCKET;
		needFireEvent = true;
	}

	if (!mIoContextConnect.mBusying)
	{
		PostDispose(mIoContextConnect.mBaseClient);
	}
	if (!mIoContextSend.mBusying)
	{
		PostDispose(mIoContextSend.mBaseClient);
	}
	if (!mIoContextRecv.mBusying)
	{
		PostDispose(mIoContextRecv.mBaseClient);
	}

	if (needFireEvent)
	{
		OnClose();
	}
}

//服务器收到连接会运行到此
int TcpClient_Windows::OnConnect(long handle, Bundle* extraInfo)
{
	ASSERT(IsMyselfThread());
	SOCKET s = (SOCKET)handle;

	if (s != INVALID_SOCKET)
	{
		ASSERT(mSock == INVALID_SOCKET);
		mSock = s;

		CheckInitTls(true);

		{
			mPeerDesc = StringTool::Format("%s:%d", SockTool::GetPeerIP(s).c_str(), SockTool::GetPeerPort(s));
			mLocalDesc =StringTool::Format("%s:%d", SockTool::GetLocalIP(s).c_str(), SockTool::GetLocalPort(s));
		}
		{
			// 设置连接的KEEPALIVE参数
			DWORD ulBytesReturn = 0;
			struct tcp_keepalive stKeepIn = { 0 }, stKeepOut = { 0 };
			stKeepIn.keepalivetime = 5*60 * 1000;		// 超过此秒数没接收数据就发送探测包
			stKeepIn.keepaliveinterval = 20 * 1000;	// 每隔几秒发送一次探测包
			stKeepIn.onoff = 1;						// 启用KEEPALIVE
			int ret = WSAIoctl(s, SIO_KEEPALIVE_VALS, (LPVOID)&stKeepIn, sizeof(tcp_keepalive), (LPVOID)&stKeepOut,
				sizeof(tcp_keepalive), &ulBytesReturn, NULL, NULL);
			if (ret == SOCKET_ERROR)
			{
				int x = 0;
			}
		}

#ifdef _MSC_VER
		HANDLE handle = CreateIoCompletionPort((HANDLE)mSock, mIocp, (ULONG_PTR)(IocpObject*)this, 0);
		ASSERT(handle == mIocp);
		if (!handle)
		{
			LogW(TAG,"fail CreateIoCompletionPort,error=%d", GetLastError());
		}

		ConfigCacheBox();
		mIoContextRecv.PostRecv();
#else
		ASSERT(FALSE);//todo
#endif
	}
	else
	{
		//crazy!
		ASSERT(FALSE);
		return -1;
	}

	SignalOnConnect(this, 0, nullptr, extraInfo);
	return 0;
}

//作为客户端工作，连接ack时会调用本接口
void TcpClient_Windows::OnConnectAck()
{
	int ret = -1;

	if (mSock != INVALID_SOCKET)
	{
#ifdef _MSC_VER
		ret = GetLastError();

		setsockopt(mSock, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
		int seconds;
		int bytes = sizeof(seconds);
		int iResult = 0;

		iResult = getsockopt(mSock, SOL_SOCKET, SO_CONNECT_TIME,
			(char *)&seconds, (PINT)&bytes);
		if (iResult != NO_ERROR)
		{
			LogW(TAG,"getsockopt(SO_CONNECT_TIME) failed with error: %u", WSAGetLastError());
		}
		else
		{
			if (seconds == 0xFFFFFFFF)
			{
				//LogW(TAG,"Connection not established yet\n");
			}
			else
			{
				//DT("Connection has been established %ld seconds\n", seconds);
				ret = 0;
			}
		}
#else
		ASSERT(FALSE);//todo
#endif

		}

	if (ret)
	{
		SignalOnConnect(this, ret, nullptr, nullptr);
	}
	else
	{
		ConfigCacheBox();
		CheckInitTls(false);
		mIoContextRecv.PostRecv();

		SignalOnConnect(this, 0, nullptr, nullptr);
	}
	}

void TcpClient_Windows::ConfigCacheBox()
{
	{
		//todo:
		//2020.01.07
		//mSslBox->setOnEncData目前没做渐进处理，
		//libiot做的微信小程序查询历史记录可能比较大,outbox要足够大能容纳下
		//后续改进
		mOutbox.SetBufferSize(4 * 1024, 16*1024 * 1024);

		/*
		2022.06.15 海康摄像机修改参数时xml长达6KB,为支持HttpGet,这里加大outbox
		*/
		mOutbox.PrepareBuf(8 * 1024);

		IoContext& context = mIoContextSend;
		context.mType = IoContextType_Send;
		context.mSock = mSock;
		context.mByteBuffer.PrepareBuf(8 * 1024);
		context.mBaseClient = dynamic_pointer_cast<TcpClient>(shared_from_this());
	}
	{
		IoContext& context = mIoContextRecv;
		context.mType = IoContextType_Recv;
		context.mSock = mSock;
		context.mByteBuffer.PrepareBuf(8 * 1024);
		context.mBaseClient = dynamic_pointer_cast<TcpClient>(shared_from_this());
	}
}

LRESULT TcpClient_Windows::OnMessage(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case BM_DESTROY:
	{
		Close();
		break;
	}
	case BM_DUMP:
	{
		int x = 0;
		break;
	}
	case BM_DNS_ACK:
	{
		string dns = (const char*)wp;
		string ip = (const char *)lp;
		//DV("%s=[%s]", dns.c_str(), ip.c_str());
		if (dns == mBundle.GetString("address") && mSock == INVALID_SOCKET)
		{
			ConnectHelper(ip);
		}
		else
		{
			//已取消连接
		}

		return 0;
	}
	}

	return __super::OnMessage(msg, wp, lp);
}

void TcpClient_Windows::MarkEndOfRecv()
{
	shutdown(mSock, SD_RECEIVE);
}

void TcpClient_Windows::MarkEndOfSend()
{
	mMarkEndOfSend = true;

	if (mOutbox.IsEmpty())
	{
		shutdown(mSock, SD_SEND);
	}
}

ByteBuffer* TcpClient_Windows::GetRawInbox()
{
#ifdef _CONFIG_OPENSSL
	if (mTlsInfo)
	{
		return &mTlsInfo->mInboxSSL;
	}
#endif

	return &mInbox;
}

#ifdef _CONFIG_OPENSSL

void TcpClient_Windows::EnableTls()
{
	if (!mTlsInfo)
	{
		mTlsInfo = make_unique<tagTlsInfo>();
	}
}
#endif

int TcpClient_Windows::GetOutboxCacheBytes()
{
	return mOutbox.length();
}

void TcpClient_Windows::CheckInitTls(bool serverMode)
{
#ifdef _CONFIG_OPENSSL
	if (mTlsInfo)
	{
		auto& obj = mTlsInfo;
		obj->mInboxSSL.PrepareBuf(1024 * 16);

		obj->mInBuffer = make_shared<BufferRaw>();
		//obj->mOutBuffer = make_shared<BufferRaw>();

		obj->mSslBox = make_shared<SSL_Box>(serverMode);
		obj->mSslBox->setOnDecData([this](const Buffer::Ptr& buffer) {
			//public_onRecv(pBuf);
			auto data = buffer->data();
			auto bytes = buffer->size();
			//DV("setOnDecData,bytes=%d", bytes);
			int ret = mInbox.Write(data, bytes);
			ASSERT(ret == bytes);
			mInbox.MakeSureEndWithNull();

			SignalOnReceive(this);
		});
		obj->mSslBox->setOnEncData([this](const Buffer::Ptr& buffer) {
			int x = 0;
			auto data = buffer->data();
			auto bytes = buffer->size();
			//DV("setOnEncData,bytes=%d", bytes);
			//public_send(buffer);
			auto ret = mOutbox.Write(data, bytes);
			ASSERT(ret == bytes);

			CheckSend();
		});

		if (!serverMode)
		{
			if (!isIP(mAddress.c_str())) {
				//设置ssl域名
				obj->mSslBox->setHost(mAddress.c_str());//服务器不能调用setHost,否则tls握手会只返回7字节导致失败
			}
		}
	}
#endif

}


}
}
}