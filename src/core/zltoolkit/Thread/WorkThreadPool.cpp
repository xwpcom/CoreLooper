﻿#include "stdafx.h"
/*
 * MIT License
 *
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <vector>
#include <iostream>
#include "WorkThreadPool.h"
#include "Util/logger.h"
#include "Util/onceToken.h"
#include "ThreadPool.h"

namespace toolkit {

int WorkThreadPool::s_pool_size = 0;

INSTANCE_IMP(WorkThreadPool);

EventPoller::Ptr WorkThreadPool::getFirstPoller(){
	return dynamic_pointer_cast<EventPoller>(_threads.front());
}

EventPoller::Ptr WorkThreadPool::getPoller(){
	return dynamic_pointer_cast<EventPoller>(getExecutor());
}

WorkThreadPool::WorkThreadPool(){
    //创建当前cpu核心个数优先级最低的线程，目的是做些无关紧要的阻塞式任务，例如dns解析，文件io等
	auto size = s_pool_size ? s_pool_size : thread::hardware_concurrency();
	createThreads([](){
		EventPoller::Ptr ret(new EventPoller(ThreadPool::PRIORITY_LOWEST));
		ret->runLoop(false, false);
		return ret;
	},size);
}

void WorkThreadPool::setPoolSize(int size) {
	s_pool_size = size;
}

} /* namespace toolkit */

