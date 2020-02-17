#pragma once
namespace Bear {
namespace Core {
//XiongWanPing 2019.09.29
//采用共享内存来做日志

class CORE_EXPORT MemoryLog
{
public:
	MemoryLog();
	virtual ~MemoryLog();

	void AddLog(const char* msg);
protected:
	int mValue = 0x12345678;
};


}
}
