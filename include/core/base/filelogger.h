#pragma once

namespace Bear {
namespace Core {
struct tagLogItem
{
	string tag;
	string msg;
	string file;
	int line = 0;
	int level = 0;
	int threadId = 0;
	tagTimeMs t;
};

/*
XiongWanPing 2022.04.01
以追加方式记录日志到文本

*/
class CORE_EXPORT LogFile :public Handler
{
	SUPER(Handler);
public:
	LogFile();
	void setFilePath(const string& filePath, int maxBytes , bool autoBackup = true);

	void addLog(const string& text)
	{
		addLog((LPBYTE)text.c_str(), (int)text.length());
	}
	void addLog(LPBYTE data, int bytes);

	sigslot::signal2<Handler*, const string&> SignalFileBackupReady;
protected:
	int write(LPBYTE data,int bytes);
	string mTag;

	string mFilePath;
	int mMaxBytes = 1024 * 512;
	bool mAutoBackup = true;
};

/*
XiongWanPing 2022.04.12
保存日志到文件
增加本功能的原因:嵌入式设备有些业务没按预期执行，需要保存到日志文件便于事后检查
flash空间有限，要控制保存文件的总大小和频次
只能创建一个实例

FileLogger功能
.在初始化之前能缓存一些日志
.初始化后能采用两个文件循环保存日志
.在析构后不再保存文件
.可指定在某个日期后自动禁用保存文件，避免耗尽flash擦写次数
.默认保存DT_DEBUG和更高级别的日志,比如LogD,LogI,LogW和LogW,要禁用的级别可用disableLevel指定
.用法见TEST_METHOD(fileLogger)
*/
class CORE_EXPORT FileLogger :public Looper
{
    SUPER(Looper);
public:
    FileLogger();
	~FileLogger();

	LOOPER_SAFE static void addLog(tagLogInfo& info);
	LOOPER_SAFE void saveLog(bool sync = false);
	LOOPER_SAFE void enableLog();
	LOOPER_SAFE void disableLog();
	LOOPER_SAFE void disableLevel(int level);//默认为DT_VERBOSE,即不保存verbose级别日志
	
	//日期晚于date后会自动禁用，date格式yyyymmdd,比如20230115
	LOOPER_SAFE void setActiveDate(int date);

	//下面的接口只应在初始化FileLogger时调用
	void addDisableTags(const string& tags);

	//循环保存日志为filePath和filePath.bak
	//限制日志文件最大字节数(可精确到字节)
	void setFilePath(const string& filePath, int maxBytes=4096,bool autoBackup = true);
	void setSaveInterval(int seconds);

protected:
    void OnCreate();
	void OnDestroy();
	void OnTimer(long id);
	void addItem(const tagLogItem& item);

	weak_ptr<LogFile> mLogFile;
	long mTimer_save = 0;
	
	long mTimer_keepAlive = 0;
	ULONGLONG mKeepAliveTick = 0;

};

}
}
