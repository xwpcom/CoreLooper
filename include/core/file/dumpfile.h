#pragma once
#include <stdio.h>

namespace Bear {
namespace Core
{
namespace FileSystem {
//XiongWanPing 2016.03.20
class CORE_EXPORT DumpFile
{
public:
	DumpFile();
	virtual ~DumpFile();

	int Open(const char *filename);
	int Open(std::string filename)
	{
		return Open(filename.c_str());
	}

	int Write(void* data, int dataLen);
	int Close();
	bool IsOpen()const
	{
		return mFile != NULL;
	}

	long GetPos();
protected:
	FILE * mFile;
};
}
}
}