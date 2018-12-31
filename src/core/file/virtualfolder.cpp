#include "stdafx.h"
#include "file/virtualfolder.h"
#include "file/file.h"
using namespace std;

namespace Bear {
namespace Core
{
namespace FileSystem {

VirtualFolder::VirtualFolder(void)
{
}

VirtualFolder::~VirtualFolder(void)
{
}

shared_ptr<tagVirtualFolderPoint> VirtualFolder::AddMount(const char *name, const char *path)
{
	auto item(make_shared<tagVirtualFolderPoint>());
	item->mName = name;
	if (item->mName.empty() || IsNameExits(item->mName))
	{
		ASSERT(FALSE);
		return nullptr;
	}

	item->mPath = path;
	mMountPoints.push_back(item);
	return item;
}

bool VirtualFolder::IsNameExits(const string& name)
{
	for (auto iter = mMountPoints.begin(); iter != mMountPoints.end(); ++iter)
	{
		if (StringTool::CompareNoCase((*iter)->mName, name) == 0)
		{
			return true;
		}
	}

	return false;
}

//把客户端虚拟绝对path映射为本地全路径
//virtualFullPathFile可能是文件夹或者文件名
string VirtualFolder::Virtual2LocalPathFile(const string& virtualFullPathFile, BOOL bSuperUser)
{
	if (bSuperUser)
	{
		return virtualFullPathFile;
	}

	//取virtualFullPathFile中的根文件夹
	char clientPath[MAX_PATH];
	CLR_BUF(clientPath);
	strncpy(clientPath, virtualFullPathFile.c_str(), sizeof(clientPath) - 1);
	if (clientPath[0] != '/')
	{
		return "";
	}
	if (clientPath[1] == 0)
	{
		return "/";
	}

	char *pszRelativePath = NULL;//相对位置
	clientPath[sizeof(clientPath) - 1] = 0;
	int len = (int)strlen(clientPath);
	for (int i = 1; i < len; i++)
	{
		if (File::IsPathSplitChar(clientPath[i]))
		{
			clientPath[i] = 0;
			pszRelativePath = clientPath + i + 1;//供后面拼接
			break;
		}
	}

	//clientPath为客户端根文件夹,与mount点比较

	//把client path转成svr mount point path
	string localPathFile;

	for (auto iter = mMountPoints.begin(); iter != mMountPoints.end(); ++iter)
	{
		string name = "/" + (*iter)->mName;
		if (StringTool::CompareNoCase(name, clientPath) == 0 && !(*iter)->mPath.empty())
		{
			if (pszRelativePath && strlen(pszRelativePath) > 0)
			{
				localPathFile = StringTool::Format("%s/%s", (*iter)->mPath.c_str(), pszRelativePath);
			}
			else
			{
				localPathFile = (*iter)->mPath;
			}
			break;
		}
	}

	return localPathFile;
}

//判断pszPathFile是否为虚拟根文件夹
//有很多操作不能用于虚拟根文件夹,比如rename,move,delete
//pszPathFile必须是全路径
BOOL VirtualFolder::IsVirtualRootDir(const char *virtualPathFile, BOOL bSuperUser)
{
	if (bSuperUser)
	{
		return FALSE;
	}

	for (auto iter = mMountPoints.begin(); iter != mMountPoints.end(); ++iter)
	{
		string name0 = "/" + (*iter)->mName;
		string name1 = "/" + (*iter)->mName + "/";
		if (StringTool::CompareNoCase(name0, virtualPathFile) == 0
			|| StringTool::CompareNoCase(name1, virtualPathFile) == 0
			)
		{
			return TRUE;
		}
	}

	return FALSE;
}
}
}
}
