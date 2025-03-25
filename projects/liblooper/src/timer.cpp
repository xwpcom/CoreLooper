#include "pch.h"
#include "timer.h"

namespace Core {

Timer::Timer(uint32_t ms, const std::function<bool()> &cb, const Looper::Ptr &poller) {
    _poller = poller;
    if (!_poller) {
        _poller = Looper::instance();
    }

    _tag = _poller->doDelayTask(ms, [cb, ms]() {
        try {
            if (cb()) {
                //Recurring task
                return (uint64_t)ms;
            }

			//This task no longer recurs
            return (uint64_t)0;
        } catch (std::exception &ex) {
            logW(__func__) << "Exception occurred when do timer task: " << ex.what();
            return (uint64_t)ms;
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
