#pragma once
#include "ftpdatahandler.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSource;
class DataSink;

class FtpDataOutHandler :public FtpDataHandler
{
	SUPER(FtpDataHandler);
public:
	FtpDataOutHandler();
	~FtpDataOutHandler();

protected:
	void OnConnect(Channel*, long error, Bundle*);
	void OnSend(Channel*);
	void OnReceive(Channel*);
	void OnClose(Channel*);

	void CheckSend();
	void SetDataSource(shared_ptr<DataSource> obj);

};
}
}
}
}