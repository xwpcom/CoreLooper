#include "stdafx.h"
#include "file/file.h"
#include "base/shelltool.h"
#include "base/stringtool.h"
#include "filefinder.h"
#include "transferfileinfo.h"
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#ifndef _CONFIG_ANDROID
#include <sys/timeb.h>
#endif
#include <sys/stat.h>

#ifdef _MSC_VER
#include <io.h>
#define access _access
#include <shlobj.h>
#elif defined __APPLE__
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/sysinfo.h>
#include <time.h>
#include <sys/time.h>
#include <utime.h>
#include <sys/vfs.h>
#endif

using namespace std;
namespace Bear {
namespace Core
{
namespace FileSystem {
FILE* File::fopen(const char *pszFile, const char *pszMode)
{
	if (!pszFile)
		return NULL;

	FILE *hFile = ::fopen(pszFile, pszMode);
	if (!hFile)
	{
#ifdef _MSC_VER
		DW("Fail to open[%s],err=%d(%s)", pszFile, errno, strerror(errno));
#endif
	}

	return hFile;
}

size_t File::GetFileLength(FILE *hFile)
{
	if (!hFile)
	{
		ASSERT(FALSE);
		return 0;
	}

	size_t nFileSize = 0;
	long pos = ftell(hFile);
	fseek(hFile, 0, SEEK_END);
	nFileSize = ftell(hFile);
	fseek(hFile, pos, SEEK_SET);
	return nFileSize;
}

size_t File::GetFileLength(const char *pszFile)
{
	if (!pszFile)
	{
		ASSERT(FALSE);
		return 0;
	}

	size_t dwLen = 0;

	struct _stat fs;
	memset(&fs, 0, sizeof(fs));
	int ret = ::_stat(pszFile, &fs);
	if (ret == 0)
	{
		dwLen = fs.st_size;
	}

	return dwLen;
}

char *File::concat_path_file(const char *path, const char *filename, char *outbuf, int cbOutBuf)
{
	ASSERT(cbOutBuf >= MAX_PATH - 1);

	if (!outbuf || cbOutBuf <= 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	memset(outbuf, 0, cbOutBuf);

	if (!path)
	{
		path = "";
	}

	//char *lc = string::last_char_is(path, '/');
	bool flag = StringTool::EndWith(path, "/");
	while (*filename == '/')
	{
		filename++;
	}

	_snprintf(outbuf, cbOutBuf - 1, "%s%s%s", path, flag ? "" : "/", filename);
	return outbuf;
}

#ifndef _MSC_VER
enum {
	FILEUTILS_PRESERVE_STATUS = 1,
	FILEUTILS_PRESERVE_SYMLINKS = 2,
	FILEUTILS_RECUR = 4,
	FILEUTILS_FORCE = 8,
	FILEUTILS_INTERACTIVE = 16
};

int File::remove_file(const char *path, int flags)
{
	DT("remove_file(%s),flags=%d", path, flags);

	struct stat path_stat;
	int path_exists = 1;

	if (lstat(path, &path_stat) < 0)
	{
		if (errno != ENOENT)
		{
			DW("unable to stat `%s'", path);
			return -1;
		}

		path_exists = 0;
	}

	if (!path_exists)
	{
		if (!(flags & FILEUTILS_FORCE))
		{
			DW("cannot remove `%s'", path);
			return -1;
		}
		return 0;
	}

	if (S_ISDIR(path_stat.st_mode))
	{
		DIR *dp;
		struct dirent *d;
		int status = 0;

		if (!(flags & FILEUTILS_RECUR))
		{
			DW("%s: is a directory", path);
			return -1;
		}

		if ((!(flags & FILEUTILS_FORCE) && access(path, W_OK) < 0 &&
			isatty(0)) ||
			(flags & FILEUTILS_INTERACTIVE)) {
			//fprintf(stderr, "%s: descend into directory `%s'? ", applet_name,path);
		}

		if ((dp = opendir(path)) == NULL)
		{
			DW("FailOpen `%s'", path);
			return -1;
		}

		while ((d = readdir(dp)) != NULL)
		{
			char new_path[MAX_PATH];

			if (strcmp(d->d_name, ".") == 0 ||
				strcmp(d->d_name, "..") == 0)
				continue;

			concat_path_file(path, d->d_name, new_path, sizeof(new_path) - 1);
			if (remove_file(new_path, flags) < 0)
				status = -1;
		}

		if (closedir(dp) < 0)
		{
			DW("FailClose '%s'", path);
			return -1;
		}

		if (flags & FILEUTILS_INTERACTIVE)
		{
			//fprintf(stderr, "%s: remove directory `%s'? ", applet_name, path);
		}

		if (rmdir(path) < 0)
		{
			DW("FailRemove '%s'", path);
			return -1;
		}

		return status;
	}
	else
	{
		if ((!(flags & FILEUTILS_FORCE) && access(path, W_OK) < 0 &&
			!S_ISLNK(path_stat.st_mode) &&
			isatty(0)) ||
			(flags & FILEUTILS_INTERACTIVE)) {
			//fprintf(stderr, "%s: remove `%s'? ", applet_name, path);
		}

		if (unlink(path) < 0)
		{
			DW("FailRemove '%s'", path);
			return -1;
		}
		//sync();

		return 0;
	}
}
#endif

int File::DeleteFile(const char *pszFile)
{
	if (pszFile && pszFile[0])
	{
		//DV("File::DeleteFile(%s)", pszFile);
	}
	else
	{
		return -1;
	}

#ifdef _MSC_VER
	BOOL bOK = ::DeleteFileA(pszFile);
	if (!bOK && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		bOK = TRUE;
	}
	return bOK ? 0 : -1;
#else
	int ret = unlink(pszFile);
	if (ret)
	{
		DW("unlink(%s) errno=%d(%s)", pszFile, errno, strerror(errno));
	}
	return ret;
#endif
}

//linux下递归删除文件夹,windows下只能删除空文件夹
int File::DeleteFolder(const char *pszFile)
{
	DV("File::DeleteFolder(%s)", pszFile);
#ifdef _MSC_VER
	BOOL bOK = ::RemoveDirectoryA(pszFile);
	if (!bOK && GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		bOK = TRUE;
	}
	return bOK ? 0 : -1;
#else
	int ret = remove_file(pszFile, FILEUTILS_RECUR | FILEUTILS_FORCE);
	return ret;
#endif
}

//检查文件或文件夹是否存在
BOOL File::FileExists(const  string& szFile)
{
#if defined _RUN_AT_PC_LINUX
	//发现ubuntu下面,access(pszFile,F_OK);不能访问/mnt/work/下面的文件
	FILE *hFile = ::fopen(szFile.c_str(), "rb");
	if (hFile)
	{
		fclose(hFile);
		return TRUE;
	}
	return FALSE;
#endif

#ifndef F_OK
#define F_OK	0
#endif

	int ret = access(szFile.c_str(), F_OK);
	if (ret)
	{
#ifndef _MSC_VER
		//DW("fail access %s,ret=%d,error=%d(%s)",pszFile,ret,errno,strerror(errno));
#endif
	}
	return ret == 0;
}

//删除指定的文件夹(不递归)
BOOL File::RemoveDirectory(const char *pszDir)
{
#ifdef _MSC_VER
	BOOL bOK = ::RemoveDirectoryA(pszDir);
	return bOK;
#else
	int ret = rmdir(pszDir);
	return ret == 0;
#endif
}

//删除pszFile的所有上级文件夹(一般在调用DeleteFile(pszFile)之后调用本函数)
//主要用于删除录像文件之后，删除空文件夹,避免留下大量没用的空文件夹
int File::DeleteParentFolder(const char *pszFile)
{
	char szFile[MAX_PATH];
	memset(szFile, 0, sizeof(szFile));
	strncpy(szFile, pszFile, sizeof(szFile) - 1);
	szFile[sizeof(szFile) - 1] = 0;
	int len = (int)strlen(szFile);
	//char ch=*FILE_PATH_SPLIT;
	for (int i = len - 1; i > 0; i--)
	{
		if (szFile[i] == '/' || szFile[i] == '\\')
		{
			szFile[i] = 0;

			BOOL bOK = RemoveDirectory(szFile);
			if (bOK)
			{
				//DT("RemoveDirectory(%s)",szFile);
			}
			else
			{
				int err = ShellTool::GetLastError();
				if (err != 0x00000091 && err != ENOTDIR && err != ENOTEMPTY)//folder is NOT empty
				{
					DW("fail RemoveDirectory(%s),err=%d(%s)", szFile, err, strerror(errno));
				}
				//maybe NOT empty?
				return -1;
			}

		}
	}

	return 0;
}

//设置文件的修改时间
int File::SetFileModifyTime(const char *szFile, time_t tm)
{
#ifdef _MSC_VER
	HANDLE hFile = ::CreateFileA(szFile, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME LastWriteTime;
		//time_t转FILETIME
		{
			FILETIME ft;
			LONGLONG ll = Int32x32To64(tm, 10000000) + 116444736000000000;
			ft.dwLowDateTime = (DWORD)ll;
			ft.dwHighDateTime = (DWORD)(ll >> 32);

			LastWriteTime = ft;
		}

		BOOL bOK = SetFileTime(hFile, NULL, NULL, &LastWriteTime);
		::CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
		return bOK ? 0 : -1;
	}

	return -1;
#else
	int ret = -1;
	//struct utimbuf times;
	//times.actime = times.modtime = tm;
	//ret=utime (szFile,&times);
	struct timeval times[2];
	memset(times, 0, sizeof(times));
	times[0].tv_sec = tm;
	times[1].tv_sec = tm;
	ret = utimes(szFile, times);

	if (ret)
	{
		DW("Fail to utime(%s),error=%d(%s)", szFile, errno, strerror(errno));
	}
	return 0;
#endif
}

int	File::rename(const char *oldname, const char *newname)
{
	if (!oldname || !newname)
	{
		ASSERT(FALSE);
		return -1;
	}

	int ret = ::rename(oldname, newname);
	if (ret)
	{
		DW("rename(%s,%s) errno=%d(%s)", oldname, newname, errno, strerror(errno));
	}

	return ret;
}

string File::GetUpperFolder(const char *pszFile)
{
	char szDir[MAX_PATH];
	memset(szDir, 0, sizeof(szDir));
	strncpy(szDir, pszFile, sizeof(szDir) - 1);
	szDir[sizeof(szDir) - 1] = 0;
	int len = (int)strlen(szDir);
	for (int i = len - 1; i > 0; i--)
	{
		if (szDir[i] == '/' || szDir[i] == '\\')
		{
			szDir[i] = 0;
			break;
		}
	}

	return szDir;
}

BOOL File::CreateFolderForFile(const string& filePath)
{
	string szDir = GetUpperFolder(filePath.c_str());
	BOOL bOK = MakeSureDirectoryPathExists(szDir.c_str());
	return bOK;
}

BOOL File::MakeSureDirectoryPathExists(LPCSTR lpszDirPath)
{
	char szDir[MAX_PATH];
	memset(szDir, 0, sizeof(szDir));
	strncpy(szDir, lpszDirPath, sizeof(szDir) - 1);
	szDir[sizeof(szDir) - 1] = 0;
	for (int nPos = 0; nPos < (int)strlen(szDir); nPos++)
	{
		if (File::IsPathSplitChar(szDir[nPos]))
		{
			szDir[nPos] = 0;

			File::mkdir(szDir, 0777);

			szDir[nPos] = '/';
		}
	}
	int ret = File::mkdir(szDir, 0777);
	return ret == 0;
}

int File::mkdir(const char *pszDir, DWORD mode)
{
	if (!pszDir || pszDir[0] == 0)
	{
		//ASSERT(FALSE);
		return -1;
	}

#ifdef _MSC_VER
	int ret = SHCreateDirectoryExA(NULL, pszDir, NULL);
	if (ret == ERROR_SUCCESS
		|| ret == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
	return -1;
#else
	int ret = ::mkdir(pszDir, mode);
	if (ret)
	{
		//可能需要创建多级dir
		char path[MAX_PATH + 1];
		memset(path, 0, sizeof(path));
		strncpy(path, pszDir, sizeof(path) - 1);
		size_t len = strlen(path);
		for (size_t i = 1; i < len; i++)
		{
			if (path[i] == '/')
			{
				char chSave = path[i];
				path[i] = 0;
				::mkdir(path, mode);
				path[i] = chSave;
			}
		}
		::mkdir(path, mode);
		DIR * dir = opendir(path);
		if (dir)
		{
			closedir(dir);
			dir = NULL;
			ret = 0;
		}
	}
#endif
	return ret;
}

int File::chmod(const char *pszFile, DWORD mode)
{
	int ret = -1;
#ifndef _MSC_VER
	ret = ::chmod(pszFile, mode);
	if (ret)
	{
		DW("chmod fail ret=%d,file=[%s],errno=%d(%s)", ret, pszFile, errno, strerror(errno));
	}
#endif
	return ret;
}

//The sync function simply queues all the modified block buffers for writing and returns;
//it does not wait for the disk writes to take place.
void File::sync()
{
#ifndef _MSC_VER
	::sync();
#endif
}

void File::PathMakePretty(string& filePath)
{
	char buf[MAX_PATH] = { 0 };
	strncpy(buf, filePath.c_str(), sizeof(buf) - 1);
	PathMakePretty(buf);
	filePath = buf;
}

//szFile:文件的绝对路径
void File::PathMakePretty(char *szFile)
{
	//把\变成/
	//DT("mkdir#1(%s)",szFile);
	{
		int len = (int)strlen(szFile);
		for (int i = 0; i < len; i++)
		{
			if (szFile[i] == '\\')
				szFile[i] = '/';
		}
	}
	//把两个//变成一个/
	{
		int nDst = 0;
		int nLen = (int)strlen(szFile);
		for (int nSrc = 0; nSrc < nLen; nSrc++)
		{
			if (szFile[nSrc] == '/' && szFile[nSrc + 1] == '/')
			{
				szFile[nDst++] = szFile[nSrc++];
			}
			else
			{
				szFile[nDst++] = szFile[nSrc];
			}
		}
		szFile[nDst] = 0;
	}
}

int File::Dump(ByteBuffer& box, const char *pszFile)
{
	int ret = -1;
	LPBYTE pData = box.GetDataPointer();
	int cbData = box.GetActualDataLength();
	if (pData && cbData >= 0)
	{
		ret = Dump(pData, cbData, pszFile);
	}

	return ret;
}

int File::Dump(string sz, const char *pszFile)
{
	return Dump((const LPVOID)sz.c_str(), (int)sz.length(), pszFile);
}

int File::Dump(const LPVOID pBuf, int nBuf, const char *pszFile)
{
	if (!pBuf || nBuf < 0 || !pszFile)
	{
		//ASSERT(FALSE);
		return -1;
	}

	int ret = -1;
	CreateFolderForFile(pszFile);

	FILE *hFile = File::fopen(pszFile, "wb");
	//ASSERT(hFile);
	if (hFile)
	{
		if (pszFile && nBuf > 0)
		{
			ret = (int)fwrite(pBuf, 1, nBuf, hFile);
			if (ret == nBuf)
			{
				ret = 0;
			}
			else
			{
				DW("fail fwriate,nBuf=%d,ret=%d", nBuf, ret);
			}
		}
		else
		{
			//DW("fail dump[%s],invalid param,nBuf=%d",pszFile,nBuf);
		}

		fclose(hFile);
		hFile = NULL;
	}
	else
	{
		DW("fail dump [%s],error=%d(%s)", pszFile, errno, strerror(errno));
	}

	return ret;
}

int File::ReadFile(const char *szFile, ByteBuffer& box)
{
	box.clear();
	FILE *hFile = File::fopen(szFile, "rb");
	if (hFile)
	{
		int len = (int)File::GetFileLength(hFile);
		if (len > 0)
		{
			auto ret = box.PrepareBuf(len + 1);
			if (ret == 0)
			{
				int bytes = (int)fread(box.GetNewDataPointer(), 1, len, hFile);
				if (bytes == len)
				{
					box.WriteDirect(len);
					box.MakeSureEndWithNull();//便于把box data当作const char*
				}
			}
		}

		fclose(hFile);
		hFile = NULL;

		return 0;
	}

	return -1;
}

void File::RemoveTailPathSplitChar(char *file)
{
	if (!file)
	{
		return;
	}

	for (int i = (int)strlen(file) - 1; i >= 0; i--)
	{
		if (IsPathSplitChar(file[i]))
		{
			file[i] = 0;
		}
		else
		{
			break;
		}
	}
}

string File::ConcatPath(const char *psz1, const char* psz2)
{
	string sz1(psz1);
	string sz2(psz2);
	string sz = sz1 + "/" + sz2;
	StringTool::Replace(sz, "//", "/");

	return sz;
}

//返回pathfile中最后一部分，用来在ftp中显示
string File::GetPathLastName(const char *pszPathFile)
{
	if (!pszPathFile)
		return "";

	int len = (int)strlen(pszPathFile);
	int i;
	for (i = len - 1; i >= 0; i--)
	{
		if (!File::IsPathSplitChar(pszPathFile[i]))
		{
			break;
		}

	}
	int endPos = i;
	for (; i >= 0; i--)
	{
		if (File::IsPathSplitChar(pszPathFile[i]))
		{
			break;
		}

	}

	string name(pszPathFile + i + 1, endPos - i);
	return name;
}

bool File::PathIsDirectory(const char *psz)
{
	if (!psz)
	{
		ASSERT(FALSE);
		return FALSE;
	}

#ifdef _MSC_VER
	BOOL bDir = ::PathIsDirectoryA(psz);
	return bDir;
#else
	DIR *dir = opendir(psz);
	if (dir)
	{
		closedir(dir);
		return TRUE;
	}
	return FALSE;
#endif
}

int File::MoveFile(const char *pszOldName, const char *pszNewName)
{
#ifdef _MSC_VER
	BOOL bOK = ::MoveFileA(pszOldName, pszNewName);
	return bOK ? 0 : -1;
#else
	//rename() renames a file, moving it between directories if required.

	int ret = rename(pszOldName, pszNewName);
	if (ret)
	{
		DW("rename(%s,%s) errno=%d(%s)", pszOldName, pszNewName, errno, strerror(errno));
	}
	return ret;
#endif
}

void File::RemoveInvalidFileNameChar(string& fileName)
{
	//windows文件名中非法字符 \ / :*? "<>|
	StringTool::Replace(fileName, "\\", "");
	StringTool::Replace(fileName, "/", "");
	StringTool::Replace(fileName, ":", "");
	StringTool::Replace(fileName, "*", "");
	StringTool::Replace(fileName, "?", "");
	StringTool::Replace(fileName, "\"", "");
	StringTool::Replace(fileName, "<", "");
	StringTool::Replace(fileName, ">", "");
	StringTool::Replace(fileName, "|", "");
}

int File::GetFileSystemInfo(const char *path, DWORD& totalKB, DWORD& freeKB)
{
	int ret = -1;

	totalKB = 0;
	freeKB = 0;
#ifdef _MSC_VER
#else

#if defined _CONFIG_HI3516 && !defined _DEBUG
	{
		string dir = path;
		if (dir.Find("/nfs") != -1)
		{
			DW("skip [%s]", dir.c_str());
			return -1;
		}
	}
#endif

	struct statfs fs = { 0 };
	ret = statfs(path, &fs);//path can be mount point,/tmp/eye
	if (ret == 0)
	{
		totalKB = (DWORD)(fs.f_bsize * fs.f_blocks / 1024);
		freeKB = (DWORD)(fs.f_bsize * fs.f_bfree / 1024);
	}
#endif

	return ret;
}

//创建临时文件
//返回文件名
string File::CreateTmpFile()
{
	char filename[MAX_PATH];
	filename[0] = 0;

#ifdef _MSC_VER
	char dir[MAX_PATH];
	dir[0] = 0;
	GetTempPathA(sizeof(dir), dir);

	int ret = GetTempFileNameA(dir, "MG.", 0, filename);
	string file(filename);
	return file;
#else
	strncpy(filename, "/tmp/MG.XXXXXX", sizeof(filename) - 1);
	int fd = mkstemp(filename);
	ASSERT(fd);
	close(fd);
	string file(filename);
	return file;
#endif
}

//从path中提取最后一个/或\后面的部分，一般为文件名+.+扩展名
string File::GetPathFileName(string path)
{
	int len = (int)path.length();
	for (int i = len - 1; i >= 0; i--)
	{
		char ch = path[i];
		if (ch == '/' || ch == '\\')
		{
			return path.substr(i + 1);
		}
	}

	return "";
}

const char * File::GetFileExt(const char *filepath)
{
	if (!filepath || filepath[0] == 0)
	{
		return "";
	}

	const char *ext = strrchr(filepath, '.');
	if (ext)
	{
		if (strchr(ext, '/') || strchr(ext, '\\'))
		{
			return NULL;
		}
	}

	return ext;
}

string File::GetFileTitle(const char *filepath)
{
	string filename = GetPathLastName(filepath);
	int idx = (int)filename.rfind('.');
	if (idx == -1)
	{
		return filename;
	}

	string title = filename.substr(0, idx);
	return title;
}

//计算文件夹和子文件夹下的所有文件占用的字节数
ULONGLONG File::CalcFolderContentBytes(string folder)
{
	ULONGLONG bytes = 0;

	FileFinder finder;
	BOOL bOK = finder.FindFile(folder.c_str());
	while (bOK)
	{
		bOK = finder.FindNextFile();
		if (finder.IsDots())
		{
			continue;
		}

		string fullPath = folder + "/" + finder.GetFileName();
		if (finder.IsDirectory())
		{
			bytes += CalcFolderContentBytes(fullPath);
		}

		bytes += File::GetFileLength(fullPath.c_str());
	}

	return bytes;
}

int File::ftruncate(int fd, off_t length)
{
#ifdef _MSC_VER
	return _chsize(fd, length);
#else
	return ::ftruncate(fd, length);
#endif
}

int File::CopyFile(const string& sourceFilePath, const string& destFilePath)
{
	if (File::FileExists(destFilePath.c_str()))
	{
		File::DeleteFile(destFilePath.c_str());
	}

	auto hDestFile = fopen(destFilePath.c_str(), "wb");
	if (!hDestFile)
	{
		DW("fail open %s", destFilePath.c_str());
		return -1;
	}
	auto destFile = shared_ptr<FILE>(hDestFile, ::fclose);

	auto hSourceFile = fopen(sourceFilePath.c_str(), "rb");
	if (!hSourceFile)
	{
		DW("fail open %s", sourceFilePath.c_str());
		return -1;
	}
	auto sourceFile = shared_ptr<FILE>(hSourceFile, ::fclose);

	char buf[4096];
	while (1)
	{
		auto ret = fread(buf, 1, sizeof(buf), hSourceFile);
		if (ret > 0)
		{
			auto bytes = fwrite(buf, 1, ret, hDestFile);
			if (bytes != ret)
			{
				DW("fwrite fail,want write %d,return %d", ret, bytes);
				return -1;
			}
		}

		if (ret < sizeof(buf))
		{
			//eof
			break;
		}
	}


	return 0;
}

//return 0表示内容相同，否则表示内容不同
int File::CompareFileContent(const string& filePath1, const string& filePath2)
{
	int ret = -1;

	auto bytes1 = GetFileLength(filePath1.c_str());
	auto bytes2 = GetFileLength(filePath2.c_str());
	if (bytes1 != bytes2)
	{
		return -1;
	}

	if (bytes1 == 0)
	{
		return 0;
	}

	ByteBuffer box1,box2;
	File::ReadFile(filePath1, box1);
	File::ReadFile(filePath2, box2);

	if (box1.length() == 0 || box1.length() != box2.length())
	{
		DW("fail read file?");
		return -1;
	}

	return memcmp(box1.data(),box2.data(),box1.length());
}

}
}
}