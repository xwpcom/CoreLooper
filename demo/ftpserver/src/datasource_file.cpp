#include "stdafx.h"
#include "datasource_file.h"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;

DataSource_File::DataSource_File()
{
	SetObjectName("DataSource_File");
}

DataSource_File::~DataSource_File()
{
	int x = 0;
}

int DataSource_File::SetFilePath(const string& filePath)
{
	if (mFile)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto file = fopen(filePath.c_str(), "rb");
	if (!file)
	{
		return -1;
	}

	mFile = shared_ptr<FILE>(file, ::fclose);

	SignalDataChanged(this);
	return 0;
}

long DataSource_File::Read(LPBYTE buf, long bytes)
{
	if (!mFile)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto f = mFile.get();
	auto ret = fread(buf, 1, bytes, f);
	if (ret <= 0)
	{
		if (feof(f))
		{
			return 0;
		}
	}

	return (long)ret;
}
