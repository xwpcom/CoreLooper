#pragma once

namespace Bear {
namespace Core2 {
using namespace std;

//loger能比logger少写一个字母
enum class LogLevel
{
	error,
	warn,
	info,
	debug,
	verbose,
};

/*
log设计参考了zlmediakit
.增加了TAG
 个人觉得TAG非常有用,必须要支持
.去掉了func
 由于我们的dt双击能自动在vs中定位到代码行,func没太大作用
*/

class LogItem : public ostringstream {
public:
	LogItem() = default;
	LogItem(const char* tag,LogLevel level, const char* file, int line);
	LogItem(const string& tag,LogLevel level, const char* file, int line);
	~LogItem() = default;

	LogLevel mLevel;
	string mFilePath;
	int mLine;
	string mTag;
	struct timeval mTime;

	const string& str();

private:
	bool mGotContent = false;
	string mContent;
};

using LogItemPtr = shared_ptr<LogItem>;

#define LogL(tag,level) Log(tag,level, __FILE__, __LINE__)

//tag能起分类过滤作用,强制要求提供tag
#define LogerV(tag)	LogL(tag,LogLevel::verbose)
#define LogerD(tag)	LogL(tag,LogLevel::debug)
#define LogerI(tag)	LogL(tag,LogLevel::info)
#define LogerW(tag)	LogL(tag,LogLevel::warn)
#define LogerE(tag)	LogL(tag,LogLevel::error)

/*
XiongWanPing 2023.01.04
被LogV(TAG,"xx",...)伤过几次，格式化和参数容易不匹配导致crash,所以core2改用ostream来做
*/
class CORE_EXPORT Log
{
public:
	Log(const char* tag,LogLevel level,const char* file,int line)
		:mItem(new LogItem(tag,level, file, line))
	{
	}

	Log(const string& tag,LogLevel level, const char* file, int line)
		:mItem(new LogItem(tag,level, file, line))
	{
	}
	
	~Log()
	{
		*this << endl;
	}

	Log& operator<<(ostream& (*f)(ostream&));

	template<typename T>
	Log& operator<<(T&& data) {
		if (mItem) {
			(*mItem) << forward<T>(data);
		}
		return *this;
	}

protected:
	LogItemPtr mItem;
};


}
}
