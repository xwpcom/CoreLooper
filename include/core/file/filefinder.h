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

/*
iot.jjyip.com/pushEvent.json?tag=ls&count=10&dir=/opt/media/mmcblk0p1/media/2026-09-30&uid=STL04427MEE

root@t31ap:/opt/media/mmcblk0p1/media/2026-09-30# ls -l
-rwxrwxrwx    1 root     root          8295 Sep 30 14:15 2026-09-30.json
-rwxrwxrwx    1 root     root          8295 Sep 30 14:15 2026-09-30.json.bak
-rwxrwxrwx    1 root     root        524288 Sep 30 03:40 2026-09-30_03_40_01.log

可看到上面remote telnet中显示的时间是正确的，但下面lsAck返回的文件时间整体后移了8小时

{
  "cid": "",
  "dir": "/opt/media/mmcblk0p1/media/2026-09-30",
  "index": 0,
  "count": 10,
  "countAck": 3,
  "total": 3,
  "d": [],
  "f": [
	{
	  "n": "2026-09-30_03_40_01.log",
	  "t": "2026-09-30 11:40:01",
	  "b": 524288
	},
	{
	  "n": "2026-09-30.json.bak",
	  "t": "2026-09-30 22:45:02",
	  "b": 8581
	},
	{
	  "n": "2026-09-30.json",
	  "t": "2026-09-30 22:45:02",
	  "b": 8581
	}
  ]
}
*/
	time_t writeTime()const
	{
		auto t = mStat.st_ctime;
#ifndef _MSC_VER
		t -= 8 * 3600;//
#endif

		return t;
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

	void setOrderAsc()
	{
		mOrderAsc = true;
	}
	void setOrderDesc()
	{
		mOrderAsc = false;
	}

protected:
	int				mIdx;
	std::string		mDir;
	std::string		mExt;//当为空时匹配所有文件类型，否则只返回匹配的后缀文件
	std::vector<tagFileFindItem>	mItems;//tagFileFindItem*
	bool			mStopOnCorruptFiles = false;
	bool			mCorrupt = false;
	bool			mFolderAtFirst = true;
	bool			mOrderAsc = true;
	static bool sortcmpFolderAtFirst(const tagFileFindItem& a, const tagFileFindItem& b);
	static bool sortcmpByName(const tagFileFindItem& a, const tagFileFindItem& b);
	static bool sortcmpFolderAtFirst_desc(const tagFileFindItem& a, const tagFileFindItem& b);
	static bool sortcmpByName_desc(const tagFileFindItem& a, const tagFileFindItem& b);
	string mTag = "fileFinder";
};

}
}
}