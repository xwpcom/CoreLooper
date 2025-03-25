#include "pch.h"
#include "file.h"

namespace Core{


void File::pathMakePretty(char* szFile)
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

void File::pathMakePretty(std::string& filePath)
{
	char buf[MAX_PATH] = { 0 };
	strncpy(buf, filePath.c_str(), sizeof(buf) - 1);
	pathMakePretty(buf);
	filePath = buf;
}

string File::parentFolder(const string& szFile)
{
	char szDir[MAX_PATH];
	memset(szDir, 0, sizeof(szDir));
	strncpy(szDir, szFile.c_str(), sizeof(szDir) - 1);

	int len = (int)strlen(szDir);
	for (int i = len - 1; i > 0; i--)
	{
		if (szDir[i] == '/' || szDir[i] == '\\')
		{
			szDir[i] = 0;
			break;
		}
	}

	string ack = szDir;
	File::pathMakePretty(ack);

	return ack;
}


}