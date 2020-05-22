#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSink :public Handler
{
	SUPER(Handler)
public:
	DataSink();
	~DataSink();

	virtual long Write(LPBYTE data, long bytes) = 0;
};
}
}
}
}