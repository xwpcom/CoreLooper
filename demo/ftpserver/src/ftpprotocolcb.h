#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class FtpProtocolCB
{
public:
	virtual ~FtpProtocolCB() {}
	virtual int Output(LPVOID data, int bytes) = 0;
	virtual int OnCommand(const string& cmd) = 0;
};
}
}
}
}