#include "stdafx.h"
#include "loger2.h"
#include "loghandler_dt.h"

namespace Bear {
using namespace Core;
namespace Core2 {

static const char* TAG = "loger";

int gettimeofday(struct timeval* tp, void* tzp) {
	auto now_stamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	tp->tv_sec = (decltype(tp->tv_sec))(now_stamp / 1000000LL);
	tp->tv_usec = now_stamp % 1000000LL;
	return 0;
}

LogItem::LogItem(const char* tag,LogLevel level, const char* file, int line)
	:mLevel(level), mLine(line), mFilePath(file), mTag(tag)
{
	gettimeofday(&mTime, nullptr);
}

LogItem::LogItem(const string& tag,LogLevel level, const char* file,int line)
	:mLevel(level), mLine(line), mFilePath(file),  mTag(tag)
{
	gettimeofday(&mTime, nullptr);
}

const string& LogItem::str() 
{
	if (!mGotContent) 
	{
		mContent = ostringstream::str();
		mGotContent = true;
	}
	
	return mContent;
}

Log& Log::operator<<(ostream& (*f)(ostream&))
{
	if (mItem) 
	{
		auto& item = *mItem;
		auto& text = item.str();
		//_logger.write(mItem);

		
		{
			LogItemInfo info;
			info.mFile = item.mFilePath.c_str();
			info.mLevel = (int)item.mLevel;
			info.mLine = item.mLine;
			info.mTag = item.mTag.c_str();
			info.msg = text.c_str();
			LogHandler_DT::send(info);
		}

		mItem.reset();
	}

	return *this;
}

}
}
