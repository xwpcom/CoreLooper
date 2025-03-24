#pragma once
#include "noncopyable.h"

class TaskCancelable : public noncopyable {
public:
	TaskCancelable() = default;
	virtual ~TaskCancelable() = default;
	virtual void cancel() = 0;
};

template<class R, class... ArgTypes>
class TaskCancelableImp;

template<class R, class... ArgTypes>
class TaskCancelableImp<R(ArgTypes...)> : public TaskCancelable {
public:
	using Ptr = std::shared_ptr<TaskCancelableImp>;
	using func_type = std::function<R(ArgTypes...)>;

	~TaskCancelableImp() = default;

	template<typename FUNC>
	TaskCancelableImp(FUNC&& task) {
		_strongTask = std::make_shared<func_type>(std::forward<FUNC>(task));
		_weakTask = _strongTask;
	}

	void cancel() override {
		_strongTask = nullptr;
	}

	operator bool() {
		return _strongTask && *_strongTask;
	}

	void operator=(std::nullptr_t) {
		_strongTask = nullptr;
	}

	R operator()(ArgTypes ...args) const {
		auto strongTask = _weakTask.lock();
		if (strongTask && *strongTask) {
			return (*strongTask)(std::forward<ArgTypes>(args)...);
		}
		return defaultValue<R>();
	}

	template<typename T>
	static typename std::enable_if<std::is_void<T>::value, void>::type
		defaultValue() {}

	template<typename T>
	static typename std::enable_if<std::is_pointer<T>::value, T>::type
		defaultValue() {
		return nullptr;
	}

	template<typename T>
	static typename std::enable_if<std::is_integral<T>::value, T>::type
		defaultValue() {
		return 0;
	}

protected:
	std::weak_ptr<func_type> _weakTask;
	std::shared_ptr<func_type> _strongTask;
};

using TaskIn = std::function<void()>;
using Task = TaskCancelableImp<void()>;
