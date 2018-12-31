#pragma once
#ifdef _MSC_VER
//XiongWanPing 2011.02.22
namespace Bear {
namespace Core {
class CORE_EXPORT Utf8Tool
{
public:
	static void UTF_8ToUnicode(wchar_t* pOut, const char *pText);  // 把UTF-8转换成Unicode   
	static void UnicodeToUTF_8(char* pOut, const wchar_t* pText);  //Unicode 转换成UTF-8   
	static void UnicodeToGB2312(char* pOut, wchar_t uData);  // 把Unicode 转换成 GB2312   
	static string UnicodeToGB2312(const wchar_t* uData);
	static void Gb2312ToUnicode(wchar_t* pOut, const char *gbBuffer);// GB2312 转换成　Unicode   
	static int Gb2312ToUnicode(string input, wchar_t* out, int outBytes);
	static void GB2312ToUTF_8(string& pOut, const char *pText, int pLen);//GB2312 转为 UTF-8   
	static string  GB2312ToUTF_8(const string& text);//GB2312 转为 UTF-8   
#ifdef _MSC_VER
	static string  GB2312ToUTF_8(const CString& text);
	static string  UTF_8ToGB2312(const string& text);
#endif

#ifdef _MSC_VER
	static CString UTF8_to_UNICODE(const char *utf8_string, int length);
	static void UNICODE_to_UTF8(const CString& unicodeString, std::string& str);
#endif


	static void UTF_8ToGB2312(string&pOut, const char *pText, int pLen);//UTF-8 转为 GB2312   
};
}
}
#endif
