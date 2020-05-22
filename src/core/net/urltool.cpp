#include "stdafx.h"
#include "urltool.h"

using namespace std;

namespace Bear {
namespace Core {
namespace Net {

string UrlTool::GetUriTitle(string url)
{
	const char *key = "//";
	auto pos = (int)url.find(key);

	if (pos != -1)
	{
		url = url.substr(pos + strlen(key));

		//eat host
		pos = (int)url.find("/");
		if (pos != -1)
		{
			url = url.substr(pos + 1);
		}
	}


	pos = (int)url.find("?");
	if (pos != -1)
	{
		url = url.substr(0,pos);
	}

	pos = (int)url.rfind('.');
	if (pos != -1)
	{
		url = url.substr(0,pos);
	}

	return url;
}

#if 0
string UrlTool::GetUriTitle(string url)
{
	const char *key = "http://";
	if (url.Find(key) == 0)
	{
		int pos = url.Find("/", strlen(key) + 1);
		if (pos != -1)
		{
			url = url.Right(url.length() - pos);
		}
	}

	int pos = url.Find('.');
	if (pos != -1)
	{
		int end = pos;// +4;//不包括后面的.xml

		int start = 0;
		char ch = url.GetAt(0);
		if (ch == '/' || ch == '\\')
		{
			start = 1;
		}

		return url.Mid(start, end - start);
	}

	return "";
}
#endif

}
}
}
