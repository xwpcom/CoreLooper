#pragma once
#include <sstream>

namespace Core {
using namespace std;

/*
value to text
用于格式化int,float,double
w用法: w=4时,对int是%04d,对float,doubel是%.4f

	auto mTag = "fmt";
	float v = 0.12345678f;
	int i = 123;
	logV(mTag) << "i=" << vt(i, 8) << ",float=" << vt(v,2);
	//结果为:
	//i=00000123,float=0.12
*/
struct vt
{
	vt(int d, uint8_t w=4) :i(d), width(w) { isInt = true; }
	vt(double d, uint8_t w=3) :f(d), width(w) { isInt = false; }
	int i=0;
	double f = 0;
	uint8_t width;
	bool isInt = true;
};

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

#define LogX(tag,level) Log(tag,level, __FILE__, __LINE__)

//tag能起分类过滤作用,强制要求提供tag
#define logV(tag)	LogX(tag,LogLevel::verbose)
#define logD(tag)	LogX(tag,LogLevel::debug)
#define logI(tag)	LogX(tag,LogLevel::info)
#define logW(tag)	LogX(tag,LogLevel::warn)
#define logE(tag)	LogX(tag,LogLevel::error)

/*
XiongWanPing 2023.01.04
被LogV(TAG,"xx",...)伤过几次，格式化和参数容易不匹配导致crash,所以core2改用ostream来做
*/
class Log
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
	
	Log& operator<<(vt&& obj) {
		if (mItem) {
			char buf[64] = { 0 };
			char fmt[16] = { 0 };
			if (obj.isInt)
			{
				snprintf(fmt, sizeof(fmt) - 1, "%%0%dd",obj.width);
				snprintf(buf, sizeof(buf) - 1, fmt,obj.i);
			}
			else
			{
				snprintf(fmt, sizeof(fmt) - 1, "%%.%df", obj.width);
				snprintf(buf, sizeof(buf) - 1, fmt, obj.f);
			}

			(*mItem) << buf;
		}
		return *this;
	}

protected:
	LogItemPtr mItem;
};
}
