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

	void AddLog(const char *msg);
};

class CORE_EXPORT MemoryLogD :public MemoryLog
{
public:
	static MemoryLog* GetInstance();

};

class CORE_EXPORT MemoryLogEx :public MemoryLog
{
public:
	MemoryLogEx();
	~MemoryLogEx();

	static MemoryLog* GetInstance();

};

}
}
