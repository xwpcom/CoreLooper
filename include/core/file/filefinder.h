#pragma once

#include <sys/stat.h>

#ifndef _MSC_VER
#define _stat32	stat
#endif

namespace Bear {
namespace Core
{
namespace FileSystem {
struct tagFileFindItem
{
	std::string		mName;
	struct _stat32	mStat;
	BOOL IsFolder()const
	{
#ifdef _MSC_VER
		return (_S_IFMT & mStat.st_mode) == _S_IFDIR;
#else
		return (S_IFMT & mStat.st_mode) == S_IFDIR;
#endif
	}

	static bool IsDirectory(const struct _stat32& stat)
	{
#ifdef _MSC_VER
		return (_S_IFMT & stat.st_mode) == _S_IFDIR;
#else
		return (S_IFMT & stat.st_mode) == S_IFDIR;
#endif
	}
};

//XiongWanPing 2011.06.15
class CORE_EXPORT FileFinder
{
public:
	FileFinder(void);
	virtual ~FileFinder(void);

	BOOL FindFile(const string&dir, std::string ext = "");
	BOOL FindNextFile();
	BOOL IsDots();
	BOOL IsDirectory();
	std::string GetFileName();
	ULONGLONG GetLength();
	void Close();
#ifdef _MSC_VER
	BOOL GetLastWriteTime(CTime& tmWrite);
#else
	BOOL GetLastWriteTime(time_t& tmWrite);
#endif

	void EnableStopOnCorruptFiles(bool enable)
	{
		mStopOnCorruptFiles = enable;
	}

	bool MaybeCorrupt()const
	{
		return mCorrupt;
	}

protected:
	int				mIdx;
	std::string		mDir;
	std::string		mExt;//当为空时匹配所有文件类型，否则只返回匹配的后缀文件
	std::vector<tagFileFindItem>	mItems;//tagFileFindItem*
	bool			mStopOnCorruptFiles = false;
	bool			mCorrupt = false;
	static bool sortcmp(const tagFileFindItem& a, const tagFileFindItem& b);
};

}
}
}