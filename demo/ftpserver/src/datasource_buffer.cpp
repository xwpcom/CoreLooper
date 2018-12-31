#include "stdafx.h"
#include "datasource_buffer.h"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;

DataSource_Buffer::DataSource_Buffer()
{
	SetObjectName("DataSource_Buffer");
}

DataSource_Buffer::~DataSource_Buffer()
{
	int x = 0;
}

void DataSource_Buffer::SetData(const string& data)
{
	mBox.clear();

	auto bytes = (int)data.length();
	if (bytes > 0)
	{
		mBox.PrepareBuf(bytes);
		mBox.Write((LPBYTE)data.c_str(), bytes);

		SignalDataChanged(this);
	}
}

long DataSource_Buffer::Read(LPBYTE buf, long bytes)
{
	auto len = mBox.GetDataLength();
	if (len == 0)
	{
		return 0;
	}

	auto eatBytes = MIN(bytes, len);
	memcpy(buf, mBox.GetDataPointer(), eatBytes);
	mBox.Eat(eatBytes);

	return eatBytes;
}
