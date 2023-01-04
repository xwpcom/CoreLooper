#include "stdafx.h"
#include "loger2.h"

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

		if (0)
		{
			if (item.mLevel == LogLevel::verbose)
			{
				LogV(mItem->mTag, "%s", text.c_str());
			}
			else if (item.mLevel == LogLevel::debug)
			{
				LogD(item.mTag, "%s", text.c_str());
			}
			else if (item.mLevel == LogLevel::info)
			{
				LogI(item.mTag, "%s", text.c_str());
			}
			else if (item.mLevel == LogLevel::warn)
			{
				LogW(item.mTag, "%s", text.c_str());
			}
			else if (item.mLevel == LogLevel::error)
			{
				LogE(item.mTag, "%s", text.c_str());
			}
		}

		mItem.reset();
	}

	return *this;
}

}
}
