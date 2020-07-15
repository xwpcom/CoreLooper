#include "stdafx.h"
#include "utf8tool.h"
#include "core/base/bytetool.h"

#ifdef _MSC_VER
namespace Bear {
namespace Core {

#ifdef _MSC_VER
//http ://www.knowsky.com/resource/gb2312tbl.htm
//GB2312简体中文编码表

//https://www.cnblogs.com/lidabo/p/3903616.html
//https ://blog.poxiao.me/p/unicode-character-encoding-conversion-in-cpp11/
CString Utf8Tool::UTF8_to_UNICODE(const char *utf8_string, int length)
{
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, utf8_string, length, NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_UTF8, NULL, utf8_string, length, wszString, wcsLen);
	wszString[wcsLen] = '\0';
	CString unicodeText(wszString);
	delete[] wszString;

	return unicodeText;
}

void Utf8Tool::UNICODE_to_UTF8(const CString& unicodeString, std::string& str)
{
	auto stringLength = ::WideCharToMultiByte(CP_UTF8, NULL, unicodeString, (int)wcslen(unicodeString), NULL, 0, NULL, NULL);

	char* buffer = new char[stringLength + 1];
	::WideCharToMultiByte(CP_UTF8, NULL, unicodeString, (int)wcslen(unicodeString), buffer, stringLength, NULL, NULL);
	buffer[stringLength] = '\0';

	str = buffer;

	delete[] buffer;
}

string Utf8Tool::UNICODE_to_UTF8(const CString& unicodeString)
{
	auto stringLength = ::WideCharToMultiByte(CP_UTF8, NULL, unicodeString, (int)wcslen(unicodeString), NULL, 0, NULL, NULL);

	char* buffer = new char[stringLength + 1];
	::WideCharToMultiByte(CP_UTF8, NULL, unicodeString, (int)wcslen(unicodeString), buffer, stringLength, NULL, NULL);
	buffer[stringLength] = '\0';

	string str = buffer;

	delete[] buffer;
	return std::move(str);
}

#endif

string Utf8Tool::Unicode2Utf8(const void* text)
{
	if (!text)
	{
		return "";
	}

	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)text, -1, NULL, 0, NULL, NULL);
	auto szUtf8 = new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)text, -1, szUtf8, len, NULL, NULL);

	string ack(szUtf8);
	delete[]szUtf8;
	szUtf8 = nullptr;
	return std::move(ack);
}

//说明:
//这些代码是从网上下载的，可能有bug

void Utf8Tool::UTF_8ToUnicode(wchar_t* pOut, const char *pText)
{
	char* uchar = (char *)pOut;

	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);

	return;
}

void Utf8Tool::UnicodeToUTF_8(char* pOut, const wchar_t* pText)
{
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后   
	char* pchar = (char *)pText;

	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);
	pOut[2] = (0x80 | (pchar[0] & 0x3F));

	return;
}

void Utf8Tool::UnicodeToGB2312(char* pOut, wchar_t uData)
{
	WideCharToMultiByte(CP_ACP, NULL, &uData, 1, pOut, sizeof(wchar_t), NULL, NULL);
	return;
}

string Utf8Tool::UnicodeToGB2312(const wchar_t* uData)
{
	char *buf = nullptr;
	auto bytes = WideCharToMultiByte(CP_ACP, NULL, uData, -1, buf, NULL, NULL, NULL);
	string text;
	if (bytes > 0)
	{
		buf = new char[bytes + 1];
		bzero(buf, bytes + 1);
		WideCharToMultiByte(CP_ACP, NULL, uData, -1, buf, bytes, NULL, NULL);
		text = buf;
		delete[]buf;
		buf = nullptr;
	}
	return text;
}

void Utf8Tool::Gb2312ToUnicode(wchar_t* pOut, const char *gbBuffer)
{
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gbBuffer, 2, pOut, 1);
	return;
}

int Utf8Tool::Gb2312ToUnicode(string input, wchar_t* out, int outBytes)
{
	auto ret = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, input.c_str(), (int)input.length(), out, outBytes);
	return ret;
}

#ifdef _MSC_VER
string Utf8Tool::GB2312ToUTF_8(const CString& text)
{
	string out;
	USES_CONVERSION;
	GB2312ToUTF_8(out, T2A(text), text.GetLength());
	return out;
}
#endif

string Utf8Tool::GB2312ToUTF_8(const string& text)
{
	string out;
	GB2312ToUTF_8(out, text.c_str(), (int)text.length());
	return out;
}

void Utf8Tool::GB2312ToUTF_8(string& pOut, const char *pText, int pLen)
{
	if (!pText || pText[0] == 0 || pLen <= 0)
	{
		return;
	}

	char buf[4];
	int nLength = pLen * 3;
	char* rst = new char[nLength];
	if (!rst)
	{
		DW("new fail");
		ASSERT(FALSE);
		return;
	}

	memset(buf, 0, 4);
	memset(rst, 0, nLength);

	int i = 0;
	int j = 0;
	while (i < pLen)
	{
		//如果是英文直接复制就可以   
		if (*(pText + i) >= 0)
		{
			rst[j++] = pText[i++];
		}
		else
		{
			wchar_t pbuffer;
			Gb2312ToUnicode(&pbuffer, pText + i);

			UnicodeToUTF_8(buf, &pbuffer);

			unsigned short int tmp = 0;
			tmp = rst[j] = buf[0];
			tmp = rst[j + 1] = buf[1];
			tmp = rst[j + 2] = buf[2];

			j += 3;
			i += 2;
		}
	}
	rst[j] = '\0';

	//返回结果   
	pOut = rst;
	delete[]rst;

	return;
}

//把unicode字符串"\\u63A5\\u6536\\u5DF2\\u5904\\u7406"转为utf8
//"-8\u7B2C0\u9879\u7684\u9879\u76EEID\u4E0E\u76D1\u7763\u7F16\u53F7\u4E0D\u5339\u914D"
//json string msg = root["msg"].as<string>();提取出的字符串不是\u,而是u前缀  
string Utf8Tool::unicodeUnescape(const string& src)
{
#define CP_UTF8                   65001       // UTF-8 translation
#define CODE_PAGE CP_UTF8 //936
	string strResult;
	int nDestStep = 0;
	auto strSource = src.c_str();
	int nLength = (int)src.length();
	if (!nLength || nLength < 6) return strResult;
	const int resultMaxBytes = nLength * 2 + 2;
	char* pResult = new char[resultMaxBytes];
	wchar_t* pWbuufer = nullptr;
	if (!pResult)
	{
		pResult = NULL;
		return strResult;
	}
	ZeroMemory(pResult, resultMaxBytes);
	//"-8\\u7B2C0\\u9879";
	for (int nPos = 0; nPos < nLength; )
	{
		char ch = strSource[nPos];
		if (ch == '\\' && strSource[nPos + 1] == 'u')
		{
			char szTemp[5];
			char szSource[5];
			ZeroMemory(szTemp, 5);
			ZeroMemory(szSource, 5);
			CopyMemory(szSource, (char*)strSource + nPos + 2, 4);
			sscanf_s(szSource, "%04X", szTemp);
			CopyMemory(pResult + nDestStep, szTemp, 4);
			nDestStep += 2;

			nPos += 6;
		}
		else
		{

			pResult[nDestStep++]=ch;
			pResult[nDestStep++] = 0;
			nPos += 1;
		}
	}

	nDestStep += 2;
	pWbuufer = new wchar_t[nDestStep];
	if (!pWbuufer)
	{
		delete[] pWbuufer;
		pWbuufer = nullptr;
		return strResult;
	}
	ZeroMemory(pWbuufer, nDestStep);
	CopyMemory(pWbuufer, pResult, nDestStep);
	delete[] pResult;
	pResult = nullptr;
	CHAR* MultPtr = nullptr;
	int MultLen = -1;
	//CODE_PAGE = 936
	MultLen = ::WideCharToMultiByte(CODE_PAGE, WC_COMPOSITECHECK, pWbuufer, -1, NULL, NULL, NULL, NULL);
	MultPtr = new CHAR[MultLen + 1];
	if (MultPtr)
	{
		ZeroMemory(MultPtr, MultLen + 1);
		::WideCharToMultiByte(CODE_PAGE, WC_COMPOSITECHECK, pWbuufer, -1, MultPtr, MultLen, NULL, NULL);
		strResult = MultPtr;
		delete[] MultPtr;
		MultPtr = nullptr;
	}
	delete[] pWbuufer;
	pWbuufer = nullptr;
	return strResult;
}

string Utf8Tool::UTF_8ToGB2312(const string& text)
{
	string ack;
	UTF_8ToGB2312(ack, text.c_str(), (int)text.length());
	return ack;
}

void Utf8Tool::UTF_8ToGB2312(string &pOut, const char *pText, int pLen)
{
	pOut = "";
	//pOut.clear();

	if (pLen == 0)
	{
		return;
	}

	int len = pLen * 2;
	char * newBuf = new char[len];
	if (!newBuf)
	{
		DW("new fail");
		ASSERT(FALSE);
		return;
	}
	memset(newBuf, 0, len);

	char Ctemp[4];
	memset(Ctemp, 0, sizeof(Ctemp));

	int i = 0;
	int j = 0;

	while (i < pLen)
	{
		if (pText[i] > 0)
		{
			newBuf[j++] = pText[i++];
		}
		else
		{
			WCHAR Wtemp;
			UTF_8ToUnicode(&Wtemp, pText + i);

			UnicodeToGB2312(Ctemp, Wtemp);

			newBuf[j] = Ctemp[0];
			newBuf[j + 1] = Ctemp[1];

			i += 3;
			j += 2;
		}
	}

	pOut = newBuf;
	delete[]newBuf;

	return;
}
}
}
#endif
