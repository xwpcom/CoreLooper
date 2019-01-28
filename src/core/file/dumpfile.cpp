#include "stdafx.h"
#include "dumpfile.h"
namespace Bear {
namespace Core
{
namespace FileSystem {

DumpFile::DumpFile()
{
	mFile = NULL;
}

DumpFile::~DumpFile()
{
	if (mFile)
	{
		Close();
	}
}

int DumpFile::Open(const char *filename)
{
	Close();

	File::CreateFolderForFile(filename);
	mFile = fopen(filename, "wb");
	return mFile ? 0 : -1;
}

int DumpFile::Write(void* data, int dataLen)
{
	int ret = -1;
	if (mFile && data && dataLen > 0)
	{
		int bytes = (int)fwrite(data, 1, dataLen, mFile);
		fflush(mFile);

		if (bytes == dataLen)
		{
			ret = 0;
		}
	}

	return ret;
}

int DumpFile::Close()
{
	if (mFile)
	{
		fclose(mFile);
		mFile = NULL;
	}

	return 0;
}

long DumpFile::GetPos()
{
	long ret = 0;
	if (mFile)
	{
		ret = ftell(mFile);
	}

	return ret;
}
}
}
}