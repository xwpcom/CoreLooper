#pragma once
#include "datasource.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSource_File :public DataSource
{
	SUPER(DataSource)
public:
	DataSource_File();
	~DataSource_File();

	int SetFilePath(const string& filePath);

	virtual long Read(LPBYTE buf, long bytes);
protected:
	shared_ptr<FILE> mFile;
};
}
}
}
}