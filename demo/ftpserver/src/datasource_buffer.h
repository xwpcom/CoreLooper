#pragma once
#include "datasource.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class DataSource_Buffer :public DataSource
{
	SUPER(DataSource)
public:
	DataSource_Buffer();
	~DataSource_Buffer();

	void SetData(const string& data);

	virtual long Read(LPBYTE buf, long bytes);
protected:
	ByteBuffer mBox;
};
}
}
}
}