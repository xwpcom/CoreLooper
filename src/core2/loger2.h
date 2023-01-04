#pragma once

namespace Bear {
namespace Core2 {
using namespace std;

//loger is simple than logger
enum class LogerLevel
{
	verbose,
	debug,
	info,
	warn,
	error,
};

#define LogerV	(Log( __FILE__, __LINE__,LogerLevel.verbose))
#define LogerD	(Log( __FILE__, __LINE__,LogerLevel.debug))
#define LogerI	(Log( __FILE__, __LINE__,LogerLevel.info))
#define LogerW	(Log( __FILE__, __LINE__,LogerLevel.warn))
#define LogerE	(Log( __FILE__, __LINE__,LogerLevel.error))

//from zlmediakit
//zlmediakit中的log不支持TAG,而我觉得TAG非常有用,必须要支持
class LogContext : public std::ostringstream {
public:
	//_file,_function改成string保存，目的是有些情况下，指针可能会失效
	//比如说动态库中打印了一条日志，然后动态库卸载了，那么指向静态数据区的指针就会失效
	LogContext() = default;
	LogContext(LogerLevel level, const char* file, const char* function, int line);
	~LogContext() = default;

	LogerLevel _level;
	int _line;
	int _repeat = 0;
	std::string _file;
	std::string _function;
	std::string _thread_name;
	struct timeval _tv;

	const std::string& str();

private:
	bool _got_content = false;
	std::string _content;
};

using LogContextPtr = std::shared_ptr<LogContext>;

#define WriteL(level) Log(level, __FILE__, __FUNCTION__, __LINE__)
#define TraceL WriteL(LogerLevel::verbose)

/*
XiongWanPing 2023.01.04
被LogV(TAG,"xx",...)伤过几次，格式化和参数容易不匹配导致crash,所以core2改用ostream来做
*/
class CORE_EXPORT Log
{
public:
	Log(LogerLevel level,const char* file,const char *func, int line)
		:mFilePath(file), mFunc(func),mLine(line), mLevel(level),
		mContext(new LogContext(level, file, func, line))
	{
	}
	Log(const Log& that):mContext(that.mContext)
	{
		const_cast<LogContextPtr&>(that.mContext).reset();
	}
	
	~Log()
	{
		*this << endl;
	}

	Log& operator<<(ostream& (*f)(ostream&));

	template<typename T>
	Log& operator<<(T&& data) {
		if (!mContext) {
			return *this;
		}
		(*mContext) << std::forward<T>(data);
		return *this;
	}

protected:
	LogContextPtr mContext;
	const char* mFilePath;
	const char* mFunc;
	int mLine;
	LogerLevel mLevel;
};


}
}
