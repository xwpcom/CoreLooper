#pragma once
#include "core/base/bytebuffer.h"
#include "core/base/object.h"
namespace Bear {
namespace Core
{
namespace Net {
enum  IoContextType
{
	IoContextType_None,
	IoContextType_Accept,
	IoContextType_Connect,
	IoContextType_Send,
	IoContextType_Recv,
};

class IocpObject;
class Loop;
class TcpServer;
class TcpClient;
class CORE_EXPORT IoContext
{
public:
	IoContext();
	virtual ~IoContext();

	virtual int PostRecv(int maxRecvBytes = 0);
	virtual int PostSend();

	bool			mSocketType = true;//否则为HANDLE
	bool			mBusying;
	IoContextType	mType;
	ByteBuffer		mByteBuffer;
	WSAOVERLAPPED	mOV;
	SOCKET			mSock;

	shared_ptr<IocpObject> mBaseServer;//用来保证io还在进行中时对象有效
	shared_ptr<IocpObject> mBaseClient;
};
}
}
}