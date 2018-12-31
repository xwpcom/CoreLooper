#include "stdafx.h"
#include "datasink_file.h"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;
DataSink_File::DataSink_File()
{

}

DataSink_File::~DataSink_File()
{

}

int DataSink_File::SetFilePath(const string& filePath)
{
	if (mFile)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto file = fopen(filePath.c_str(), "wb");
	if (!file)
	{
		return -1;
	}

	mFile = shared_ptr<FILE>(file, ::fclose);
	return 0;
}

long DataSink_File::Write(LPBYTE buf, long bytes)
{
	if (!mFile)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto f = mFile.get();
	auto ret = fwrite(buf, 1, bytes, f);
	return (long)ret;
}
