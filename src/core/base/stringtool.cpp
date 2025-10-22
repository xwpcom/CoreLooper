#include "stdafx.h"
#include "stringtool.h"
#include <stdarg.h>
using namespace std;

namespace Bear {
namespace Core
{
static const char* TAG = "StringTool";

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
		
		/*
		2021.06.04,ubuntu20下面出现
terminate called after throwing an instance of 'std::out_of_range'
  what():  basic_string::erase: __pos (which is 16) > this->size() (which is 11)
vsnprintf的返回值比result.size()要大
已解决,重新每次引用fmt,list后要重新va_start
在windows下不需要，fmt,list一直有效，但ubuntu环境下面fmt,list引用后会失效
		*/
		va_end(list);
		va_start(list, fmt);
		ret = vsnprintf((char*)result.c_str(), ret + 1, fmt, list);


		//auto len = MIN(ret, result.size());
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

		va_end(list);
		va_start(list, fmt);

		ret = vsnprintf((char*)result.c_str(), ret + 1, fmt, list);
		result.erase(ret);
	}

	va_end(list);

	obj += result;
	return obj;
}

bool StringTool::StartWith(const std::string& obj, const std::string& key)
{
	if (obj.empty() || key.empty())
	{
		return false;
	}

	auto ret = strncmp(obj.c_str(), key.c_str(),key.length());
	return ret == 0;
}

bool StringTool::EndWith(const string&obj, const string& tail)
{
	if (tail.empty())
	{
		return false;
	}

	size_t len = tail.length();
	if (obj.length() < len)
	{
		return false;
	}

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
		off = pos + dstLen;
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

void StringTool::makeHexPretty(string& hex)
{
	StringTool::Replace(hex, "\r", "");
	StringTool::Replace(hex, "\n", "");
	StringTool::Replace(hex, " ", "");
	StringTool::Replace(hex, "\t", "");
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

int StringTool::ExtractInts(const string& src, char chSep, std::vector<int>& items)
{
	items.clear();

	vector<string> texts;
	ExtractSubString(src, chSep, texts);
	for (auto& item : texts)
	{
		items.push_back(atoi(item.c_str()));
	}

	return 0;
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
	return 0;
}
#endif

/*
XiongWanPing 2022.09.22
m4版字符串帮助类
注意不能使用heap,全部只能使用静态buffer
*/
int StringTool::AppendText(char* dst, int dstBytes, const char* text)
{
	if (!dst || !text)
	{
		LogW(TAG, "invaid %s", __func__);
		return -1;
	}

	auto len = strlen(dst);
	auto textLen = strlen(text);
	if (len + textLen + 1 >= (size_t)dstBytes)
	{
		LogW(TAG, "fail %s", __func__);
		return -1;
	}

	strncpy(dst + len, text, textLen);
	return 0;

}

int StringTool::AppendFormat(char* dst, int dstBytes, const char* fmt, ...)
{
	char buf[1024] = { 0 };

	va_list plist;
	va_start(plist, fmt);
	int ret = vsnprintf(buf, sizeof(buf), fmt, plist);
	va_end(plist);

	return AppendText(dst, dstBytes, buf);

}

void StringTool::split(const char* text, const char* sep, char* itemBuf, size_t itemBufBytes, function<void(const char* item)> fn)
{
	if (!text || !sep || !itemBuf || itemBufBytes <= 1)
	{
		return;
	}

	const auto sepBytes = strlen(sep);
	if (sepBytes == 0)
	{
		return;
	}

	auto s = text;
	while (s && s[0])
	{
		auto p = strstr(s, sep);
		if (p)
		{
			auto bytes = p - s;
			bytes = MIN(bytes, (int)itemBufBytes - 1);
			if (bytes > 0)
			{
				strncpy(itemBuf, s, bytes);
				itemBuf[bytes] = 0;
				fn(itemBuf);
			}

			s = p + sepBytes;
		}
		else
		{
			fn(s);
			break;
		}
	}
}

//从pSrc中读取cbDst个十六进制数据到pDst
//如果pSrc中的数据不足以填足pDst,则对应的pDst数据会以0代替.
int StringTool::HexCharToByte(const char* pSrc, unsigned char* pDst, int cbDst)
{
	int i = 0;
	if (!pSrc || !pDst)
	{
		return 0;
	}

	memset((char*)pDst, 0, cbDst);
	int ackBytes = 0;
	for (i = 0; i < cbDst; i++)
	{
		//每个hex由pSrc中的两个hex字符组成
		if (pSrc[0] && pSrc[1])
		{
			char ch0 = tolower(pSrc[0]);
			char ch1 = tolower(pSrc[1]);

			int nHigh = 0;
			int nLow = 0;
			int value = 0;

			if (ch0 >= '0' && ch0 <= '9')
				nHigh = ch0 - '0';
			else if (ch0 >= 'a' && ch0 <= 'f')
				nHigh = ch0 - 'a' + 10;

			if (ch1 >= '0' && ch1 <= '9')
				nLow = ch1 - '0';
			else if (ch1 >= 'a' && ch1 <= 'f')
				nLow = ch1 - 'a' + 10;

			value = ((nHigh << 4) | nLow);
			//ASSERT(value <= 255);

			pDst[i] = (unsigned char)value;
			++ackBytes;
		}
		else
		{
			break;
		}

		pSrc += 2;
	}

	return ackBytes;
}

int StringTool::ByteToHexChar(const unsigned char* pByte, int cbByte, char* dest, int destBytes)
{
	char* d = dest;
	int i = 0;
	d[0] = 0;

	if (cbByte * 2 >= destBytes)//确保dest以'\0'结尾
	{
		return 0;
	}

	for (i = 0; i < cbByte; i++)
	{
		char buf[8];
		int value = pByte[i];
		sprintf(buf, "%02X", value);

		d[0] = buf[0];
		d[1] = buf[1];
		d += 2;
	}

	d[0] = 0;//结尾加'\0'

	return cbByte * 2;
}

}
}
