#pragma once

#include "looper/handler.h"
#include "looper/loop.h"

namespace Bear {
namespace Core {
namespace FileSystem {
struct tagVirtualFolderPoint
{
	std::string mName;	//virtual folder name
	std::string mPath;	//real folder path
	//注意:不要与linux根目录下的顶级文件夹重名,否则可能引起误操作
	//如果name与windows本地磁盘根目录下的文件或者linux根目录顶级文件重名,可能会产生问题

	std::shared_ptr<Object> mUserObject;//用户可扩展
};

//XiongWanPing 2011.06.14
//给ftp,http提供虚拟目录支持
class CORE_EXPORT VirtualFolder :public Handler
{
public:
	std::shared_ptr<tagVirtualFolderPoint> AddMount(const char *name, const char *path);

	std::string GetVirtualRootPath()
	{
		return "/";
	}

	VirtualFolder(void);
	virtual ~VirtualFolder(void);

	BOOL IsVirtualRootDir(const char *pszPathFile, BOOL bSuperUser = FALSE);
	std::string Virtual2LocalPathFile(const std::string& virtualFullPathFile, BOOL bSuperUser = FALSE);

	std::list<std::shared_ptr<tagVirtualFolderPoint>>& GetMountPoints()
	{
		return mMountPoints;
	}
protected:
	bool IsNameExits(const std::string& name);
	std::list<std::shared_ptr<tagVirtualFolderPoint>> mMountPoints;
};
}
}
}