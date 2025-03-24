#pragma once
#include <functional>
#include "looper.h"

namespace Core {

class Timer {
public:
    using Ptr = std::shared_ptr<Timer>;

    /**
     * 构造定时器
     * @param second 定时器重复秒数
     * @param cb 定时器任务，返回true表示重复下次任务，否则不重复，如果任务中抛异常，则默认重复下次任务
     * @param poller Looper对象，可以为nullptr
     * Constructs a timer
     * @param second Timer repeat interval in seconds
     * @param cb Timer task, returns true to repeat the next task, otherwise does not repeat. If an exception is thrown in the task, it defaults to repeating the next task
     * @param poller Looper object, can be nullptr
     
     * [AUTO-TRANSLATED:7dc94698]
     */
    Timer(float second, const std::function<bool()> &cb, const Looper::Ptr &poller);
    ~Timer();

private:
    std::weak_ptr<Looper::DelayTask> _tag;
    //定时器保持Looper的强引用  [AUTO-TRANSLATED:d171cd2f]
    //Timer keeps a strong reference to Looper
    Looper::Ptr _poller;

	//string mTag = "timer";
};

}