#pragma once
#include "list.h"
#include "task.executor.h"
#include "pipeWrap.h"

namespace Core {

class Looper:public enable_shared_from_this<Looper>
{
public:
	Looper();
	virtual ~Looper();

	typedef enum {
		Event_Read	= 1 << 0,
		Event_Write = 1 << 1,
		Event_Error = 1 << 2,
		Event_LT	= 1 << 3,
	} PollEvent;

	using Ptr = shared_ptr<Looper>;
	using PollEventCB = std::function<void(int event)>;
	using PollCompleteCB = std::function<void(bool success)>;
	using DelayTask = TaskCancelableImp<uint64_t(void)>;

	int addEvent(int fd, int event, PollEventCB cb);
	int delEvent(int fd, PollCompleteCB cb = nullptr);
	int modifyEvent(int fd, int event, PollCompleteCB cb = nullptr);

	virtual Task::Ptr async(TaskIn task, bool may_sync = true);
	virtual Task::Ptr async_first(TaskIn task, bool may_sync = true);

	bool isCurrentThread();
	static Looper::Ptr currentLooper();
	thread::id getThreadId() const;
	const string& getThreadName() const;

	DelayTask::Ptr doDelayTask(uint64_t delay_ms, std::function<uint64_t()> task);

	void runLoop();
protected:
	Task::Ptr async_l(TaskIn task, bool may_sync = true, bool first = false);

	void onPipeEvent(bool flush = false);
	uint64_t flushDelayTask(uint64_t now);
	uint64_t getMinDelay();
	void addEventPipe();
	
	PipeWrap _pipe;

	bool _exit_flag=false;
	int _event_fd = -1;
	std::unordered_map<int, std::shared_ptr<PollEventCB> > _event_map;
	std::unordered_set<int> _event_cache_expired;
	std::multimap<uint64_t, DelayTask::Ptr> _delay_task_map;//timer

	std::mutex _mtx_task;
	List<Task::Ptr> _list_task;

	std::thread* _loop_thread = nullptr;
	//Logger::Ptr _logger;
	string mTag = "looper";
};

}
