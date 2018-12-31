#include "stdafx.h"
#include "file/filefinder.h"
#include "base/stringtool.h"
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
BOOL FileFinder::FindFile(const char *dir, string ext)
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
	string target = mDir + filter;
	BOOL bOK = finder.FindFile(A2T(target.c_str()));
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

		string pathfile = T2A(finder.GetFilePath());
		struct _stat32 dstat;
		int ret = _stat32(pathfile.c_str(), &dstat);
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
			item.mName = T2A(finder.GetFileName());
			item.mStat = dstat;
			mItems.push_back(item);
		}
	}
#else
	DIR * pDir = opendir(dir);
	//DT("opendir(%s)=0x%x",mDir.c_str(),pDir);
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
			File::concat_path_file(dir, entry->d_name, fullname, sizeof(fullname) - 1);
			struct stat dstat;
			int ret = lstat(fullname, &dstat);
			if (ret == 0)
			{
				if (tagFileFindItem::IsDirectory(dstat) || ext.empty())
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
		std::sort(mItems.begin(), mItems.end(), sortcmp);
	}

	mIdx = -1;
	return mItems.size() > 0;
}

bool FileFinder::sortcmp(const tagFileFindItem& a, const tagFileFindItem& b)
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
}
}
