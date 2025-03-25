#include "pch.h"
#include "looper.h"
#include "sockutil.h"

#if defined(HAS_EPOLL)
#include <wepoll/sys/epoll.h>

#define EPOLL_SIZE 512 //1024

#define toEpoll(event)        (((event) & Event_Read)  ? EPOLLIN : 0) \
                            | (((event) & Event_Write) ? EPOLLOUT : 0) \
                            | (((event) & Event_Error) ? (EPOLLHUP | EPOLLERR) : 0) \
                            | (((event) & Event_LT)    ? 0 : EPOLLET)

#define toPoller(epoll_event)     (((epoll_event) & (EPOLLIN | EPOLLRDNORM | EPOLLHUP)) ? Event_Read   : 0) \
                                | (((epoll_event) & (EPOLLOUT | EPOLLWRNORM)) ? Event_Write : 0) \
                                | (((epoll_event) & EPOLLHUP) ? Event_Error : 0) \
                                | (((epoll_event) & EPOLLERR) ? Event_Error : 0)
#define create_event() epoll_create(EPOLL_SIZE)
#endif //HAS_EPOLL

namespace Core {
static Looper* gInstance = nullptr;
Looper::Ptr Looper::instance()
{
	if (gInstance)
	{
		return gInstance->shared_from_this();
	}

	return nullptr;
}

Looper::~Looper()
{
	if (gInstance == this)
	{
		gInstance = nullptr;
	}
}

Looper::Looper()
{
	if (!gInstance)
	{
		gInstance = this;
	}

	#if defined(HAS_EPOLL)
	_event_fd = create_event();
	if (_event_fd == -1) {
		//throw runtime_error(StrPrinter << "Create event fd failed: " << get_uv_errmsg());
	}
	SockUtil::setCloExec(_event_fd);
	#endif //HAS_EPOLL

	//_name = std::move(name);
	//_logger = Logger::Instance().shared_from_this();
	addEventPipe();
}

void Looper::addEventPipe() {
	SockUtil::setNoBlocked(_pipe.readFD());
	SockUtil::setNoBlocked(_pipe.writeFD());

	if (addEvent(_pipe.readFD(), Looper::Event_Read, [this](int event) { onPipeEvent(); }) == -1) {
		throw std::runtime_error("Add pipe fd to poller failed");
	}
}


Task::Ptr Looper::async(TaskIn task, bool may_sync) {
	return async_l(std::move(task), may_sync, false);
}

Task::Ptr Looper::async_first(TaskIn task, bool may_sync) {
	return async_l(std::move(task), may_sync, true);
}

Task::Ptr Looper::async_l(TaskIn task, bool may_sync, bool first) {
	//
	if (may_sync && isCurrentThread()) {
		task();
		return nullptr;
	}

	auto ret = std::make_shared<Task>(std::move(task));
	{
		lock_guard<mutex> lck(_mtx_task);
		if (first) {
			_list_task.emplace_front(ret);
		}
		else {
			_list_task.emplace_back(ret);
		}
	}
	//写数据到管道,唤醒主线程  [AUTO-TRANSLATED:2ead8182]
	//Write data to the pipe and wake up the main thread
	_pipe.write("", 1);
	return ret;
}

bool Looper::isCurrentThread() {
	return !_loop_thread || _loop_thread->get_id() == this_thread::get_id();
}

void Looper::runLoop() 
{
#if defined(HAS_EPOLL)
	struct epoll_event events[EPOLL_SIZE];
	while (!_exit_flag) {
		auto ms = (int)getMinDelay();
		int ret = epoll_wait(_event_fd, events, EPOLL_SIZE, ms ? ms : -1);
		if (ret <= 0) {
			//Timed out or interrupted
			continue;
		}

		_event_cache_expired.clear();

		for (int i = 0; i < ret; ++i) {
			struct epoll_event& ev = events[i];
			int fd = ev.data.fd;
			if (_event_cache_expired.count(fd)) {
				//event cache refresh
				continue;
			}

			auto it = _event_map.find(fd);
			if (it == _event_map.end()) {
				epoll_ctl(_event_fd, EPOLL_CTL_DEL, fd, nullptr);
				continue;
			}
			auto cb = it->second;
			try {
				(*cb)(toPoller(ev.events));
			}
			catch (std::exception& ex) {
				logE(mTag)<< "Exception occurred when do event task: " << ex.what();
			}
		}
	}

#endif
}

uint64_t Looper::flushDelayTask(uint64_t now_time) {
	decltype(_delay_task_map) task_copy;
	task_copy.swap(_delay_task_map);

	for (auto it = task_copy.begin(); it != task_copy.end() && it->first <= now_time; it = task_copy.erase(it)) {
		//已到期的任务  [AUTO-TRANSLATED:849cdc29]
		//Expired tasks
		try {
			auto next_delay = (*(it->second))();
			if (next_delay) {
				//可重复任务,更新时间截止线  [AUTO-TRANSLATED:c7746a21]
				//Repeatable tasks, update deadline
				_delay_task_map.emplace(next_delay + now_time, std::move(it->second));
			}
		}
		catch (std::exception& ex) {
			logE(mTag) << "Exception occurred when do delay task: " << ex.what();
		}
	}

	task_copy.insert(_delay_task_map.begin(), _delay_task_map.end());
	task_copy.swap(_delay_task_map);

	auto it = _delay_task_map.begin();
	if (it == _delay_task_map.end()) {
		//没有剩余的定时器了  [AUTO-TRANSLATED:23b1119e]
		//No remaining timers
		return 0;
	}
	//最近一个定时器的执行延时  [AUTO-TRANSLATED:2535621b]
	//Delay in execution of the last timer
	return it->first - now_time;
}

uint64_t Looper::getMinDelay() {
	auto it = _delay_task_map.begin();
	if (it == _delay_task_map.end()) {
		return 0;
	}

	auto now = GetTickCount64();
	uint64_t ms = 0;
	if (it->first > now) {
		//All tasks have not expired
		ms = it->first - now;
	}
	else
	{
		//Execute expired tasks and refresh sleep delay
		ms = flushDelayTask(now);
	}
	
	//ms = min(ms, 0x7FFFFFFFULL);//wepoll.c内部已做处理,即使转为int32_t也影响不大
	return ms;
}

Looper::DelayTask::Ptr Looper::doDelayTask(uint64_t delay_ms, function<uint64_t()> task) {
	DelayTask::Ptr ret = std::make_shared<DelayTask>(std::move(task));
	auto tick = GetTickCount64() + delay_ms;
	async_first([tick, ret, this]() {
		//The purpose of asynchronous execution is to refresh the sleep time of select or epoll
		_delay_task_map.emplace(tick, ret);
				});
	return ret;
}

int Looper::addEvent(int fd, int event, PollEventCB cb) {
	
	if (!cb) {
		logW(mTag) << "PollEventCB is empty";
		return -1;
	}

	if (isCurrentThread()) {
		#if defined(HAS_EPOLL)
		struct epoll_event ev = { 0 };
		ev.events = toEpoll(event);
		ev.data.fd = fd;
		int ret = epoll_ctl(_event_fd, EPOLL_CTL_ADD, fd, &ev);
		if (ret != -1) {
			_event_map.emplace(fd, std::make_shared<PollEventCB>(std::move(cb)));
		}
		return ret;
		#endif
	}

	async([this, fd, event, cb]() mutable {
		addEvent(fd, event, std::move(cb));
		  });
	return 0;
}

int Looper::delEvent(int fd, PollCompleteCB cb) {
	
	if (!cb) {
		cb = [](bool success) {};
	}

	if (isCurrentThread()) {
		#if defined(HAS_EPOLL)
		int ret = -1;
		if (_event_map.erase(fd)) {
			_event_cache_expired.emplace(fd);
			ret = epoll_ctl(_event_fd, EPOLL_CTL_DEL, fd, nullptr);
		}
		cb(ret != -1);
		return ret;
		#endif //HAS_EPOLL
	}

	//跨线程操作  [AUTO-TRANSLATED:4e116519]
	//Cross-thread operation
	async([this, fd, cb]() mutable {
		delEvent(fd, std::move(cb));
		  });
	return 0;
}

int Looper::modifyEvent(int fd, int event, PollCompleteCB cb) {
	
	if (!cb) {
		cb = [](bool success) {};
	}
	if (isCurrentThread()) {
		#if defined(HAS_EPOLL)
		struct epoll_event ev = { 0 };
		ev.events = toEpoll(event);
		ev.data.fd = fd;
		auto ret = epoll_ctl(_event_fd, EPOLL_CTL_MOD, fd, &ev);
		cb(ret != -1);
		return ret;
		#endif // HAS_EPOLL
	}
	async([this, fd, event, cb]() mutable {
		modifyEvent(fd, event, std::move(cb));
		  });
	return 0;
}

void Looper::onPipeEvent(bool flush) {
	char buf[1024];
	int err = 0;
	if (!flush) {
		for (;;) {
			if ((err = _pipe.read(buf, sizeof(buf))) > 0) {
				// 读到管道数据,继续读,直到读空为止  [AUTO-TRANSLATED:47bd325c]
				//Read data from the pipe, continue reading until it's empty
				continue;
			}
			if (err == 0 || get_uv_error(true) != UV_EAGAIN) {
				// 收到eof或非EAGAIN(无更多数据)错误,说明管道无效了,重新打开管道  [AUTO-TRANSLATED:5f7a013d]
				//Received eof or non-EAGAIN (no more data) error, indicating that the pipe is invalid, reopen the pipe
				logW(mTag) << "Invalid pipe fd of event poller, reopen it";
				delEvent(_pipe.readFD());
				_pipe.reOpen();
				addEventPipe();
			}
			break;
		}
	}

	decltype(_list_task) _list_swap;
	{
		lock_guard<mutex> lck(_mtx_task);
		_list_swap.swap(_list_task);
	}

	_list_swap.for_each([&](const Task::Ptr& task) {
		try {
			(*task)();
		}
		catch (std::exception& ex) {
			logW(mTag) << "Exception occurred when do async task: " << ex.what();
		}
	});
}

}
