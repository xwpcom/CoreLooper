#include "stdafx.h"
#include "filelogger.h"

namespace Bear {
namespace Core {

//应该尽量避免使用静态变量,但FileLogger比较特殊
static weak_ptr<FileLogger> gWObj;
static int gDisableLevel = DT_VERBOSE;
static weak_ptr<Looper> gWorker;
static string gFilePath;
static int gMaxBytes = 1024 * 256;
static bool gAutoBackup = true;
static bool gEnable = true;
static int gActiveDate = 0;//yyyymmdd,>0时限制只在此日期之前启用,比如在2023.01.31之前启用日志，此后自动禁用日志功能,避免频繁写flash
static set<string> gDisabledTags;
static string mTag = "FileLogger";
static list<tagLogItem> gItems;
static int gMaxCacheCount = 256;//没初始化之前允许缓存的最大条数
static mutex gMutex;
static bool gDestroyed = false;
static int gSaveInterval = 5 * 60;//单位:秒,定时保存日志文件的时间

FileLogger::FileLogger()
{
    mTag = "FileLogger";
    SetObjectName("FileLogger");
}

FileLogger::~FileLogger()
{
	//方便打断点，确保能正常析构
}

void FileLogger::setFilePath(const string& filePath, int maxBytes, bool autoBackup)
{
	gFilePath = filePath;
	gMaxBytes = MAX(maxBytes, 1024);//为提高性能，做必要限制
	gAutoBackup = autoBackup;
}

LogFile::LogFile()
{
    SetObjectName("LogFile");
    mTag = "LogFile";
}

//返回成功写入的字节数
int LogFile::write(LPBYTE data, int bytes)
{
	auto fileBytes = (int)File::GetFileLength(gFilePath.c_str());
	if (fileBytes >= gMaxBytes)
	{
		if (gAutoBackup)
		{
			auto file = gFilePath + ".bak";
			File::CopyFile(gFilePath, file);

			SignalFileBackupReady(this, file);
		}

		File::DeleteFile(gFilePath.c_str());
		fileBytes = 0;
	}

	int maxWriteBytes = (gMaxBytes - fileBytes);
	int eat = MIN(bytes, maxWriteBytes);

	int ret = 0;
	if (eat > 0)
	{
		auto file = shared_ptr<FILE>(fopen(gFilePath.c_str(), "a+b"), ::fclose);
		if (file)
		{
			ret = fwrite(data, 1, eat, file.get());
		}
	}

	return ret;
}

void LogFile::addLog(LPBYTE data, int bytes)
{
	ASSERT(IsMyselfThread());

	const int bytesPerWrite= 4 * 1024;
	while (bytes > 0)
	{
		int perBytes = MIN(bytes, bytesPerWrite);
		auto eat = write(data, perBytes);
		if (eat <= 0)
		{
			break;
		}

		data += eat;
		bytes -= eat;
	}
}

void LogFile::setFilePath(const string& filePath, int maxBytes, bool autoBackup)
{
    gMaxBytes = maxBytes;
    gAutoBackup = autoBackup;

}

void FileLogger::addItem(const tagLogItem& item)
{
	ASSERT(IsMyselfThread());

	gItems.push_back(item);
	if ((int)gItems.size() >= gMaxCacheCount)
	{
		saveLog();
	}
}

void FileLogger::setSaveInterval(int seconds)
{
	gSaveInterval = MAX(seconds, 60);
}

void FileLogger::OnCreate()
{
    __super::OnCreate();
	gWObj = dynamic_pointer_cast<FileLogger>(shared_from_this());

	LogV(mTag, "%s", __func__);

	auto obj = make_shared<LogFile>();
	AddChild(obj);

	mLogFile = obj;
	obj->setFilePath(gFilePath, gMaxBytes,gAutoBackup);

	int seconds = gSaveInterval;

	SetTimer(mTimer_save, seconds * 1000);
}

void FileLogger::OnDestroy()
{
	__super::OnDestroy();

	gWObj.reset();
	gDestroyed = true;
	bool sync = true;
	saveLog(sync);

}

void FileLogger::OnTimer(long id)
{
	if (id == mTimer_save)
	{
		saveLog();
		return;
	}

	__super::OnTimer(id);
}

void FileLogger::setActiveDate(int date)
{
    gActiveDate = date;
    //LogV(mTag, "%s(%d)", __func__, date);
}

void FileLogger::enableLog()
{
    gEnable = true;
}

void FileLogger::disableLog()
{
    gEnable = false;
}

void FileLogger::saveLog(bool sync)
{
	auto fn = [this]() {
		auto obj = mLogFile.lock();
		if (obj)
		{
			ByteBuffer box;
			for (auto& item : gItems)
			{
				string levelDesc;
				if (item.level == 0)
				{
					levelDesc = "###Error  ";
				}
				else if (item.level == 1)
				{
					levelDesc = "###Warning";
				}
				auto text = StringTool::Format("%s[%s.%03d#%05d]%s#[%s]\t\t(%s:%d)\r\n"
					, levelDesc.c_str()
					, item.t.stdDateTimeText().c_str()
					, item.t.ms
					, item.threadId
					, item.tag.c_str()
					, item.msg.c_str()
					, item.file.c_str()
					, item.line
				);
				box.Write(text);
			}

			if (!box.empty())
			{
				obj->addLog(box.data(), box.length());
			}
		}

		gItems.clear();
	};

    if (sync)
    {
        sendRunnable(fn);
    }
    else
    {
        post(fn);
    }
}

//禁用一些tag,采用,分隔
void FileLogger::addDisableTags(const string& tags)
{
	auto items = StringParam::ParseItems(tags);
	for (auto& item : items)
	{
		gDisabledTags.insert(item.first);
	}
}

void FileLogger::disableLevel(int level)
{
	gDisableLevel = level;
}

static shared_ptr<FileLogger> instance()
{
	return gWObj.lock();
}

//addLog内部不要调用LogX,避免死循环
void FileLogger::addLog(tagLogInfo& info)
{
	if (!gEnable || !info.mTag || !info.msg
		|| info.mLevel >= gDisableLevel
		|| info.mTag == mTag //避免保存FileLogger自己的日志
		)
	{
		return;
	}

	if (gActiveDate)
	{
		auto t = tagTimeMs::now();
		if (t.date() > gActiveDate)
		{
			gEnable = false;
			LogV(mTag, "disable due to gActiveDate=%d", gActiveDate);
			return;
		}
	}

	auto it = gDisabledTags.find(info.mTag);
	if (it != gDisabledTags.end())
	{
		return;
	}

	tagLogItem item;
	item.tag = info.mTag;
	item.msg = info.msg;
	item.level = info.mLevel;
	item.file = info.mFile;
	item.line = info.mLine;
	item.threadId = info.tid;
	item.t = info.time;

	auto obj = instance();
	if (obj)
	{
		obj->post([obj, item]() {
				obj->addItem(item);
			});
	}
	else if(!gDestroyed)
	{
		//FileLogger没初始化时也缓存一些数据

		lock_guard<mutex> lck(gMutex);
		if (gItems.size() > (size_t)gMaxCacheCount)
		{
			gItems.pop_front();
		}

		gItems.push_back(item);
	}
}

}
}
