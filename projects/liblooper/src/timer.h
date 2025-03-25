#pragma once
#include <functional>
#include "looper.h"

namespace Core {

class Timer {
public:
    using Ptr = std::shared_ptr<Timer>;

    /**
     * 构造定时器
     * @param ms 定时器重复ms秒
     * @param cb 定时器任务，返回true表示重复下次任务，否则不重复，如果任务中抛异常，则默认重复下次任务
     * @param poller Looper对象，可以为nullptr
     */
    Timer(uint32_t ms, const std::function<bool()> &cb, const Looper::Ptr &poller=nullptr);
    ~Timer();

private:
    std::weak_ptr<Looper::DelayTask> _tag;

	//Timer keeps a strong reference to Looper
    Looper::Ptr _poller;
};

}