#pragma once
#if defined _MSC_VER

namespace Core {

struct LogItemInfo
{
	const char* mTag = nullptr;
	const char* mFile = nullptr;
	int mLine = 0;
	int mLevel = 0;

	const char* msg = nullptr;
};

/*
XiongWanPing 2023.01.05
打印日志到DT.exe
DT表示DebugTrace
DT的功能:
.根据app,tag,level过滤
.采用不同颜色显示各level
.双击能在vs中定位到日志代码行
*/
class LogHandler_DT
{
public:
	LogHandler_DT();
//protected:
	static void send(LogItemInfo& info);
};

}
#endif
