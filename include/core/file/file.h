#pragma once

//#include "base/stringex.h"
#include "core/base/bytebuffer.h"

#ifdef _MSC_VER
#define FILE_PATH_SPLIT		"\\"
#define FILE_PATH_SPLIT_CHAR	'\\'
#include <io.h>
#else
#define FILE_PATH_SPLIT		"/"
#define FILE_PATH_SPLIT_CHAR	'/'

#define _stat32	stat
#define _stat	stat

#endif
namespace Bear {
namespace Core
{
namespace FileSystem {
using namespace std;
//XiongWanPing 2008~2012
class CORE_EXPORT File
{
public:
	File();
	virtual ~File();

	int Open(const string& filePath, bool createIfNecessary = true);
	int Read(LPBYTE data, int bytes);
	int Read(void* data, int bytes)
	{
		return Read((LPBYTE)data, bytes);
	}
	int Write(LPBYTE data, int bytes);
	int Write(void* data, int bytes)
	{
		return Write((LPBYTE)data, bytes);
	}

	int Write(const string& text)
	{
		return Write((LPBYTE)text.c_str(), (int)text.length());
	}

	int Flush();

	UINT GetFileSize();
	void Close();

	int Seek(int offset, int from);
	ULONG ftell();
	bool IsEOF();
	int ftruncateEx(ULONG length);

	bool IsOpen()const
	{
		return !!mFileHandle;
	}

	static string GetFileExt(const string& fileName);
protected:
	FILE* mFileHandle = nullptr;

public:
	class CAutoClose
	{
	public:
		CAutoClose(int fd)
		{
			m_fd = fd;
			m_pFile = NULL;
		}
		CAutoClose(FILE* ps)
		{
			m_pFile = ps;
			m_fd = -1;
		}
		~CAutoClose()
		{
			if (m_pFile)
			{
				fclose(m_pFile);
				m_pFile = NULL;
			}
			if (m_fd != -1)
			{
				ASSERT(m_fd);
#ifdef _MSC_VER
				_close(m_fd);
#else
				close(m_fd);
#endif
				m_fd = -1;
			}
		}
		FILE* Detach()
		{
			ASSERT(m_pFile);
			FILE *s = m_pFile;
			m_pFile = NULL;
			return s;
		}
		int DetachFd()
		{
			ASSERT(m_fd != -1);
			int fd = m_fd;
			m_fd = -1;
			return fd;
		}
	protected:
		FILE * m_pFile;
		int		m_fd;
	};

	static int CopyFile(const string& sourceFilePath, const string& destFilePath);
	static int ftruncateEx(int fd, off_t length);

	static void RemoveInvalidFileNameChar(std::string& fileName);
	static int GetFileSystemInfo(const char *path, DWORD& totalKB, DWORD& freeKB);
	static std::string CreateTmpFile();
	static int MoveFile(const char *pszOldName, const char *pszNewName);
	static std::string GetPathLastName(const char *pszPathFile);
	static std::string GetPathLastName(const std::string& pathFile)
	{
		return GetPathLastName(pathFile.c_str());
	}
	static bool PathIsDirectory(const string& sz)
	{
		return PathIsDirectory(sz.c_str());
	}
	static bool PathIsDirectory(const char *psz);
	static char *concat_path_file(const char *path, const char *filename, char *outbuf, int cbOutBuf);
#ifdef _MSC_VER
	static int GetFileWriteTime(const string& filePath, tagTimeMs& t);
	
#else
	static int remove_file(const char *path, int flags);
#endif

	static int ReadFile(const char *szFile, ByteBuffer& box, bool autoRemoveUtf8BOM=true);
	static int ReadFile(const std::string& szFile, ByteBuffer& box,bool autoRemoveUtf8BOM=true)
	{
		return ReadFile(szFile.c_str(), box, autoRemoveUtf8BOM);
	}
	static int ReadRawFile(const std::string& szFile, ByteBuffer& box)
	{
		bool autoRemoveUtf8BOM = false;
		return ReadFile(szFile.c_str(), box, autoRemoveUtf8BOM);
	}
	static void RemoveTailPathSplitChar(char *file);
	static BOOL IsPathSplitChar(char ch)
	{
		return ch == '\\' || ch == '/';
	}
	static std::string ConcatPath(const char *psz1, const char* psz2);
	static FILE* fopen(const char *pszFile, const char *pszMode);
	static size_t GetFileLength(FILE *hFile);
	static size_t GetFileLength(const char *pszFile);
	static size_t Length(const string& filePath)
	{
		return GetFileLength(filePath.c_str());
	}
	static int Delete(const string& filePath, bool recursive = false);
	static int DeleteFile(const char *pszFile);
	static int DeleteFolder(const char *pszFile, bool recursive=false);
	static BOOL RemoveDirectory(const char *pszDir);
	static BOOL MakeSureDirectoryPathExists(LPCSTR lpszDirPath);
	static BOOL CreateFolderForFile(const std::string& filePath);
	static int DeleteParentFolder(const char *pszFile);
	static std::string GetUpperFolder(const char *pszFile);
	static BOOL FileExists(const std::string& szFile);
	static void PathMakePretty(char *szFile);
	static void PathMakePretty(std::string& filePath);
	static void sync();
	static int  mkdir(const char *pszDir, DWORD mode = 0777);
	static int  mkdir(const string& dir, DWORD mode = 0777)
	{
		return mkdir(dir.c_str(), mode);
	}
	static int  chmod(const char *pszFile, DWORD mode = 0777);
	static int	rename(const char *oldname, const char *newname);
	static int	rename(const std::string& oldname, const std::string& newname)
	{
		return rename(oldname.c_str(), newname.c_str());
	}
	static int  SetFileModifyTime(const char *szFile, time_t tm);
	static int Dump(const LPVOID pBuf, int nBuf, std::string szFile)
	{
		return Dump(pBuf, nBuf, szFile.c_str());
	}
	static int Dump(const LPVOID pBuf, int nBuf, const char *pszFile);
	static int Dump(ByteBuffer& box, const char *pszFile);
	static int Dump(ByteBuffer& box, const std::string&szFile)
	{
		return Dump(box, szFile.c_str());
	}
	static int Dump(const std::string&szFile, ByteBuffer& box )
	{
		return Dump(box, szFile.c_str());
	}
	static int Dump(std::string sz, const string& filePath)
	{
		return Dump(sz, filePath.c_str());
	}
	static int Dump(std::string sz, const char *pszFile);
	static const char *GetFileExt(const char *filepath);
	static string fileExt(const string& filePath);
	static std::string GetPathFileName(std::string path);
	static std::string GetFileTitle(const char *filepath);
	static ULONGLONG CalcFolderContentBytes(std::string folder);

	static int CompareFileContent(const string& filePath1, const string& filePath2);
};

}
}
}