#include "pch.h"
#include "timer.h"

namespace Core {

Timer::Timer(float second, const std::function<bool()> &cb, const Looper::Ptr &poller) {
    _poller = poller;
    if (!_poller) {
        //_poller = Looper::instance().getPoller();
    }
    _tag = _poller->doDelayTask((uint64_t) (second * 1000), [cb, second]() {
        try {
            if (cb()) {
                //重复的任务  [AUTO-TRANSLATED:2d440b54]
                //Recurring task
                return (uint64_t) (1000 * second);
            }
            //该任务不再重复  [AUTO-TRANSLATED:4249fc53]
            //This task no longer recurs
            return (uint64_t) 0;
        } catch (std::exception &ex) {
            logW(__func__) << "Exception occurred when do timer task: " << ex.what();
            return (uint64_t) (1000 * second);
        }
    });
}

Timer::~Timer() {
    auto tag = _tag.lock();
    if (tag) {
        tag->cancel();
    }
}

}  // namespace toolkit
