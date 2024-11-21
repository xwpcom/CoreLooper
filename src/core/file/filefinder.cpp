#include "stdafx.h"
#include "file/filefinder.h"
#include "base/stringtool.h"
#include "string/utf8tool.h"

using namespace std;

namespace Bear {
namespace Core
{
namespace FileSystem {
FileFinder::FileFinder(void)
{
	mIdx = -1;
}

FileFinder::~FileFinder(void)
{
	Close();
}

void FileFinder::Close()
{
	mItems.clear();
	mIdx = -1;
}

//为对file/folder排序，一次性读取所有item
BOOL FileFinder::FindFile(const string& dir, string ext)
{
	Close();

	mDir = dir;
	mExt = ext;
	const auto extLen = ext.length();
	//mItems.reserve(200);// .SetSize(0, 200);

#ifdef _MSC_VER
	CFileFind	finder;
	string filter;
	if (ext.empty())
	{
		filter = "\\*.*";
	}
	else
	{
		filter = "\\*" + ext;
	}

	USES_CONVERSION;
	auto target = mDir + filter.c_str();
	StringTool::Replace(target,"/", "\\");
	StringTool::Replace(target, "\\\\", "\\");
	auto wDir = Utf8Tool::UTF8_to_UNICODE(target.c_str(), target.length());
	BOOL bOK = finder.FindFile(wDir);
	if (!bOK)
	{
		return FALSE;
	}

	while (bOK)
	{
		bOK = finder.FindNextFile();

		if (finder.IsDots())
		{
			continue;
		}

		CString szFile = finder.GetFilePath();
		{
			//auto fileName = finder.GetFileName();

			//string file = Utf8Tool::UNICODE_to_UTF8(fileName);
			//string file = W2A(psz);
			//LogV("file", "file=%s", file.c_str());
			//int x = 0;
		}
		struct _stat32 dstat;
		int ret = _wstat32(szFile, &dstat);
		//int ret = _stat32(pathfile.c_str(), &dstat);
		if (ret == 0)
		{
			if (tagFileFindItem::IsDirectory(dstat) || ext.empty())
			{
			}
			else
			{
				string fileName = T2A(finder.GetFileName());
				//if(ext != fileName.Right(extLen))
				if (!StringTool::EndWith(fileName, ext))
				{
					continue;
				}
			}

			//tagFileFindItem* pItem=new tagFileFindItem;
			tagFileFindItem item;
			auto fileName = finder.GetFileName();
			//item.mName = W2A(fileName);
			{
				string file = Utf8Tool::UNICODE_to_UTF8(fileName);
				//LogV("file", "%s", file.c_str());
				item.mName = file;
			}

			item.mStat = dstat;
			mItems.push_back(item);
		}
		else
		{
			int x = 0;
		}
	}
#else
	DIR * pDir = opendir(dir.c_str());
	//LogV(mTag,"opendir(%s)=0x%x",mDir.c_str(),pDir);
	if (!pDir)
	{
		return FALSE;
	}

	{
		//m_direntSave=readdir(pDir);

		struct dirent *entry = NULL;
		while ((entry = readdir(pDir)) != NULL)
		{
			if (entry->d_name[0] == '.')
			{
				if ((!entry->d_name[1] || (entry->d_name[1] == '.' && !entry->d_name[2])))
				{
					continue;
				}
			}

			if (mStopOnCorruptFiles)
			{
				string name = entry->d_name;
				auto nc = (int)name.length();
				for (int i = 0; i < nc; i++)
				{
					auto ch = name[i];
					if (!isprint(ch))
					{
						mCorrupt = true;
						closedir(pDir);
						pDir = NULL;
						return FALSE;
					}
				}
			}

			char fullname[MAX_PATH];
			File::concat_path_file(dir.c_str(), entry->d_name, fullname, sizeof(fullname) - 1);
			struct stat dstat = {0};
			int ret = lstat(fullname, &dstat);
			bool link = (entry->d_type == DT_LNK);

			if (ret == 0 || link)
			{
				if (link || tagFileFindItem::IsDirectory(dstat) || ext.empty())
				{
				}
				else
				{
					string fileName = entry->d_name;
					if (ext != StringTool::Right(fileName,extLen))
					{
						continue;
					}
				}

				if (link)
				{
					char pathname[MAX_PATH] = { 0 };
					snprintf(pathname, sizeof(pathname) - 1, "%s/%s", dir.c_str(), entry->d_name);
					//LogV(mTag, "pathname=[%s]", pathname);
					char buf[MAX_PATH] = { 0 };
					ssize_t ret = readlink(pathname, buf, sizeof(buf) - 1);
					struct stat s;
					if (lstat(buf, &s) == 0) {
						if (S_ISDIR(s.st_mode)) {
							dstat.st_mode = S_IFDIR;
							//LogV(mTag, "is dir:[%s]",pathname);
						}
						else if (S_ISREG(s.st_mode)) {
							//LogV(mTag, "is file:[%s]", pathname);
						}
						else if (S_ISLNK(s.st_mode)) {
							//LogV(mTag, "is link:[%s]", pathname);
						}
					}
				}

				tagFileFindItem item;
				item.mName = entry->d_name;
				item.mStat = dstat;
				mItems.push_back(item);
			}
		}
		closedir(pDir);
		pDir = NULL;
	}
#endif

	{
		//auto nc = mItems.size();// GetSize();
		if (mFolderAtFirst)
		{
			std::sort(mItems.begin(), mItems.end(), sortcmpFolderAtFirst);
		}
		else
		{
			std::sort(mItems.begin(), mItems.end(), sortcmpByName);
		}
	}

	mIdx = -1;
	return mItems.size() > 0;
}

bool FileFinder::sortcmpFolderAtFirst(const tagFileFindItem& a, const tagFileFindItem& b)
{
	int ret = 0;
	if ((a.IsFolder() && b.IsFolder())
		|| (!a.IsFolder() && !b.IsFolder())
		)
	{
		//both are folders or both are files
		//ret = _stricmp(pa->mName.c_str(),pb->mName.c_str());
		ret = StringTool::CompareNoCase(a.mName, b.mName);
	}
	else
	{
		//one is folder,another is file
		ret = a.IsFolder() ? -1 : 1;
	}

	return ret > 0 ? false : true;
}

bool FileFinder::sortcmpByName(const tagFileFindItem& a, const tagFileFindItem& b)
{
	auto ret=StringTool::CompareNoCase(a.mName, b.mName);
	return ret > 0 ? false : true;
}

BOOL FileFinder::FindNextFile()
{
	int nc = (int)mItems.size();
	if (mIdx < nc - 1)
	{
		mIdx++;
	}
	else
	{
		ASSERT(FALSE);
	}

	return mIdx < nc - 1;
}

BOOL FileFinder::IsDirectory()
{
	if (mIdx != -1)
	{
		return mItems[mIdx].IsFolder();
	}

	ASSERT(FALSE);
	return FALSE;
}

BOOL FileFinder::IsDots()
{
	//在FindFile()中已经屏蔽了dots目录
	return FALSE;
}

string FileFinder::GetFileName()
{
	if (mIdx != -1)
	{
		return mItems[mIdx].mName;
	}

	ASSERT(FALSE);
	return "";
}

ULONGLONG FileFinder::GetLength()
{
	if (mIdx != -1)
	{
		return mItems[mIdx].mStat.st_size;
	}
	ASSERT(FALSE);
	return 0;
}

#ifdef _MSC_VER
BOOL FileFinder::GetLastWriteTime(time_t& tmWrite)
{
	if (mIdx != -1)
	{
		CTime t = CTime(mItems[mIdx].mStat.st_ctime);
		tmWrite = t.GetTime();
		return TRUE;
	}
	ASSERT(FALSE);
	return FALSE;
	//return finder.GetLastWriteTime(tmWrite);
}

BOOL FileFinder::GetLastWriteTime(CTime& tmWrite)
{
	if (mIdx != -1)
	{
		tmWrite = CTime(mItems[mIdx].mStat.st_ctime);
		return TRUE;
	}
	ASSERT(FALSE);
	return FALSE;
	//return finder.GetLastWriteTime(tmWrite);
}

#else
BOOL FileFinder::GetLastWriteTime(time_t& tmWrite)
{
	if (mIdx != -1)
	{
		//tagFileFindItem* pItem = (tagFileFindItem*)mItems[mIdx];
		tmWrite = mItems[mIdx].mStat.st_ctime;
		return TRUE;
	}
	ASSERT(FALSE);
	return FALSE;
}
#endif
}

/*
int FileFinder::enumFiles(function<int(const char* name, size_t bytes)> fn, const char* folder, const char* ext)
{
	return -1;
}

int FileFinder::enumFolders(function<int(const char* name)> fn, const char* folder)
{
	return -1;
}
*/

}
}
