#include "pch.h"
#include "loghandler_dt.h"
#include "bytebuffer.h"
#if defined _MSC_VER
#include<windows.h>
#include "system.h"

namespace Core {

LogHandler_DT::LogHandler_DT()
{
}

enum eType
{
	//有些项目是固定长度，所以不需要用TLV表示
	//version,版本号,每次修改WM_COPYDATA数据格式后增加版本号
	//eLevel//固定1 bytes
	//ePid,	//固定4 bytes
	//eTid,	//固定4 bytes
	//eLine,//固定4 bytes
	//date:DWORD,20200205
	//time:DWORD,hhmmssMMM

	//eEncode,//目前还没用到,todo:字符编码，比如utf8,gb2312
	eAppName = 1,
	eTag = 2,
	eMsg = 3,
	eFile = 4,
};

static char gAppName[64];
static WORD gAppNameBytes;
static void initAppName()
{
	char buf[MAX_PATH];
	::GetModuleFileNameA(nullptr, buf, sizeof(buf));
	auto pos = strrchr(buf, '\\');
	if (pos)
	{
		++pos;
		strncpy_s(gAppName, pos, sizeof(gAppName) - 1);
		{
			auto pos = strchr(gAppName, '.');
			if (pos)
			{
				//sometimes append a 'D' in debug version app name
				//for example: release version is BearStudio.exe
				//debug version is BearStudioD.exe
				//for simplify remove 'D' now
				{
					if (pos > gAppName + 1)
					{
						if (pos[-1] == 'D' && islower(pos[-2]))
						{
							pos[-1] = 0;
						}
					}
				}

				*pos = 0;
			}
		}

		gAppNameBytes = (WORD)strlen(gAppName);
	}
}

static const auto* gTitle = L"DT2020 ";
void LogHandler_DT::send(LogItemInfo& info)
{
	static HWND hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
	if (!IsWindow(hwnd))
	{
		hwnd = ::FindWindowEx(NULL, NULL, NULL, gTitle);
	}
	const char* msg = info.msg;

	if (!gAppName[0])
	{
		initAppName();
	}

	auto pid = currentPid();
	auto tid = currentTid();

	auto t = tagTimeMs::now();
	DWORD date = t.date();
	DWORD time = t.time() * 1000 + t.ms;

	ByteBuffer box;

	//static length fields
	BYTE version = 1;
	box.WriteByte(version);
	box.WriteByte((BYTE)info.mLevel);
	box.Write(&pid, sizeof(pid));
	box.Write(&tid, sizeof(tid));
	box.Write(&info.mLine, sizeof(info.mLine));
	box.Write(&date, sizeof(date));
	box.Write(&time, sizeof(time));

	//TLV fields
	//T:1 bytes
	//L:2 bytes
	{
		box.WriteByte((BYTE)eAppName);
		box.Write((int)gAppNameBytes);
		box.Write(gAppName, gAppNameBytes);
	}

	if (info.mTag)
	{
		box.WriteByte((BYTE)eTag);

		box.Write((int)strlen(info.mTag));
		box.Write(info.mTag);
	}

	{
		box.WriteByte((BYTE)eMsg);
		int len = (int)strlen(msg);
		box.Write(len);
		box.Write(msg);
	}

	{
		box.WriteByte((BYTE)eFile);
		box.Write((int)strlen(info.mFile));
		box.Write(info.mFile);
	}

	DWORD_PTR dwRet = 0;

	COPYDATASTRUCT cs;
	cs.dwData = 0;
	cs.cbData = box.length();
	cs.lpData = box.data();
	::SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&cs, SMTO_BLOCK, 10 * 1000, (PDWORD_PTR)&dwRet);
}


}
#endif
