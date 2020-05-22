#pragma once
#include "datasink.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSink_File :public DataSink
{
	SUPER(DataSink)
public:
	DataSink_File();
	virtual ~DataSink_File();

	int SetFilePath(const string& filePath);
	virtual long Write(LPBYTE buf, long bytes);

protected:
	shared_ptr<FILE> mFile;
};

}
}
}
}