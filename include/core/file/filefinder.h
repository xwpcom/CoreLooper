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

	time_t writeTime()const
	{
		return mStat.st_ctime;
	}
};

//XiongWanPing 2011.06.15
class CORE_EXPORT FileFinder
{
public:
	FileFinder(void);
	virtual ~FileFinder(void);

	//static int enumFiles(function<int(const char* name, size_t bytes)> fn, const char* folder, const char* ext = "");
	//static int enumFolders(function<int(const char* name)> fn, const char* folder);

	BOOL FindFile(const string&dir, std::string ext = "");
	BOOL FindNextFile();
	BOOL IsDots();
	BOOL IsDirectory();
	std::string GetFileName();
	ULONGLONG GetLength();
	void Close();

	void enableSoryByName()
	{
		mFolderAtFirst = false;
	}
#ifdef _MSC_VER
	BOOL GetLastWriteTime(CTime& tmWrite);
	BOOL GetLastWriteTime(time_t& tmWrite);
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

	const std::vector<tagFileFindItem>& items()
	{
		return mItems;
	}

protected:
	int				mIdx;
	std::string		mDir;
	std::string		mExt;//当为空时匹配所有文件类型，否则只返回匹配的后缀文件
	std::vector<tagFileFindItem>	mItems;//tagFileFindItem*
	bool			mStopOnCorruptFiles = false;
	bool			mCorrupt = false;
	bool			mFolderAtFirst = true;
	static bool sortcmpFolderAtFirst(const tagFileFindItem& a, const tagFileFindItem& b);
	static bool sortcmpByName(const tagFileFindItem& a, const tagFileFindItem& b);
	string mTag = "fileFinder";
};

}
}
}