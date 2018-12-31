#pragma once
#include "ftpdatahandler.h"
namespace Bear {
namespace Core {
namespace Net {
class Channel;
namespace Ftp {
class DataSource;
class DataSink;

class FtpDataInHandler :public FtpDataHandler
{
	SUPER(FtpDataHandler);
public:
	FtpDataInHandler();
	~FtpDataInHandler();

	void SetDataSource(shared_ptr<DataSource> obj);
	void SetDataSink(shared_ptr<DataSink> obj);
	void CheckSend();

	void OnDataChanged(DataSource *source);

protected:
	void OnConnect(Channel*, long error, Bundle*);
	void OnSend(Channel*);
	void OnReceive(Channel*);
	void OnClose(Channel*);
};
}
}
}
}