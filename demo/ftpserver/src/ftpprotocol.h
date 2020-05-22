#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class FtpProtocolCB;

//XiongWanPing 2018.04.23
//解析处理ftp命令协议
class FtpProtocol
{
public:
	FtpProtocol(FtpProtocolCB * cb);
	void OnConnect();
	int Input(const char *data, int bytes);
protected:
	int Output(const char *ack);
	int Output(LPVOID data, int bytes);

	int ParseInbox();

	FtpProtocolCB * mCB = nullptr;
	ByteBuffer mInbox;
};
}
}
}
}