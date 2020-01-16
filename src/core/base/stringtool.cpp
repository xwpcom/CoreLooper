#include "stdafx.h"
#include "stringtool.h"
#include <stdarg.h>
using namespace std;

namespace Bear {
namespace Core
{

string StringTool::Format(const char* fmt, ...)
{
	if (!fmt || fmt[0] == 0)
	{
		return "";
	}

	string result;
	va_list list;
	va_start(list, fmt);

	int ret = 0;
	ret = vsnprintf(nullptr, 0, fmt, list);
	if (ret >0)
	{
		result.resize(ret + 1);
		ret = vsnprintf((char*)result.c_str(), ret + 1, fmt, list);
		result.erase(ret);
	}

	va_end(list);

	return result;
}

string& StringTool::AppendFormat(string& obj, const char* fmt, ...)
{
	if (!fmt || fmt[0] == 0)
		return obj;

	string result;
	va_list list;
	va_start(list, fmt);

	int ret = 0;
	ret = vsnprintf(nullptr, 0, fmt, list);
	if (ret > 0)
	{
		result.resize(ret + 1);
		ret = vsnprintf((char*)result.c_str(), ret + 1, fmt, list);
		result.erase(ret);
	}

	va_end(list);

	obj += result;
	return obj;
}

bool StringTool::EndWith(const string&obj, const string& tail)
{
	if (tail.empty())
	{
		return false;
	}

	auto len = tail.length();
	auto text = obj.substr(obj.length() - len);
	ASSERT(text.length() == tail.length());
	return text == tail;
}

int StringTool::CompareNoCase(const string&obj1, const string& obj2)
{
	//stricmp遇到空字符串时会crash
	if (obj1.empty() || obj2.empty())
	{
		if (obj1.empty() && obj2.empty())
			return 0;
		if (!obj1.empty())
			return 1;
		return -1;
	}

#ifdef _MSC_VER
	int ret = ::_stricmp(obj1.c_str(), obj2.c_str());
#elif defined _CONFIG_SIM_CHIP
	//sim没有stricmp和strcasecmp
	{
		/*
		XiongWanPing 2020.01.15,发现sim868环境下有个奇怪的现象
		当下面采用s1 = obj1;s2=obj2时,MakeUpper会把obj1和obj2变成大写
		应该是s1直接使用了obj1的buffer并在上面做了修改
		*/

		string s1 = obj1.c_str();
		string s2 = obj2.c_str();

		MakeUpper(s1);
		MakeUpper(s2);
		ret = s1.compare(s2);
	}
#else
	int ret = strcasecmp(obj1.c_str(), obj2.c_str());
#endif

	if (ret > 1)
		ret = 1;
	else if (ret < -1)
		ret = -1;
	return ret;
}

//把obj中的src全部替换为target
string& StringTool::Replace(string&obj, const string& src, const string& dst)
{
	auto srcLen = src.length();
	auto dstLen = dst.length();
	if (srcLen == 0)
	{
		return obj;
	}

	int off = 0;
	while (1)
	{
		auto pos = obj.find(src, off);
		if (pos == string::npos)
		{
			break;
		}

		obj.replace(pos, srcLen, dst);
		off = (int)(pos + (dstLen - srcLen) + 1);
	}

	return obj;
}

string& StringTool::Trim(string&obj, char ch)
{
	TrimLeft(obj, ch);
	TrimRight(obj, ch);

	return obj;
}

string& StringTool::TrimLeft(string&obj, char ch)
{
	auto pos = obj.find_first_not_of(ch);
	if (pos != string::npos)
	{
		obj = obj.substr(pos);
	}

	return obj;
}

string& StringTool::TrimRight(string&obj, char ch)
{
	auto pos = obj.find_last_not_of(ch);
	if (pos != string::npos)
	{
		obj = obj.substr(0, pos + 1);
	}

	return obj;
}

string& StringTool::MakeUpper(string&obj)
{
	std::transform(obj.begin(), obj.end(), obj.begin(), (int(*)(int))toupper);
	return obj;
}

string& StringTool::MakeLower(string&obj)
{
	std::transform(obj.begin(), obj.end(), obj.begin(), (int(*)(int))tolower);
	return obj;
}

bool StringTool::IsHexChar(char ch)
{
	return (ch >= '0' && ch <= '9') || (ch >= 'a'&&ch <= 'f') || (ch >= 'A'&&ch <= 'F');
}

bool StringTool::IsHexString(const char *sz)
{
	if (sz == nullptr || sz[0] == 0)
	{
		return false;
	}
	int len = (int)strlen(sz);
	if (len & 1)
	{
		return false;
	}

	for (int i = 0; i < len; i++)
	{
		if (!IsHexChar(sz[i]))
		{
			return false;
		}
	}

	return true;
}

int StringTool::ExtractSubString(const char *pszSource, char chSep, vector<string>& arrString)
{
	if (!pszSource || pszSource[0] == 0)
	{
		return 0;
	}

	const char *ps = pszSource;
	int len = (int)strlen(pszSource);
	for (int i = 0; i < len; i++)
	{
		char ch = pszSource[i];
		if (ch == chSep)
		{
			const char *pe = pszSource + i;
			if (ps == pe)
			{
				ps = pe + 1;
				continue;
			}
			while (ps < pe)
			{
				if (*ps == chSep)
				{
					ps++;
				}
				else
				{
					break;
				}
			}

			if (ps != pe)
			{
				string sz(ps, (int)(pe - ps));
				arrString.push_back(sz);

				ps = pe + 1;
			}
		}
	}

	if (ps && ps[0])
	{
		string sz = ps;
		arrString.push_back(sz);
	}

	return (int)arrString.size();
}

string StringTool::Right(const string& obj, long bytes)
{
	return obj.substr(obj.length() - bytes, bytes);
}

const char *StringTool::stristr(const char *psz0, const char *psz1)
{
#ifdef _MSC_VER
	return StrStrIA(psz0, psz1);
#else
	return strcasestr(psz0, psz1);
#endif
}

string StringTool::xml(string sz)
{
	StringTool::Replace(sz, "&", "&amp;");
	StringTool::Replace(sz, "<", "&lt;");
	StringTool::Replace(sz, ">", "&gt;");
	StringTool::Replace(sz, "\"", "&quot;");
	StringTool::Replace(sz, "\'", "&apos;");
	return sz;
}

#ifdef _DEBUG
int StringTool::Test()
{
	{
		string sz = "##";
		StringTool::Replace(sz, "#", "##");
		ASSERT(sz == "####");
	}

	{
		string sz = ".";
		StringTool::Replace(sz, ".", "#");
		ASSERT(sz == "#");
	}

	{
		string sz = "192.168.1.3";
		StringTool::Replace(sz, ".", ",");
		ASSERT(sz == "192,168,1,3");
	}

	return 0;
}
#endif

}
}
