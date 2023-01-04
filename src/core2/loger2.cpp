#include "stdafx.h"
#include "loger2.h"

namespace Bear {
using namespace Core;
namespace Core2 {

static const char* TAG = "loger";

static inline const char* getFileName(const char* file) {
	auto pos = strrchr(file, '/');
#ifdef _WIN32
	if (!pos) {
		pos = strrchr(file, '\\');
	}
#endif
	return pos ? pos + 1 : file;
}

static inline const char* getFunctionName(const char* func) {
#ifndef _WIN32
	return func;
#else
	auto pos = strrchr(func, ':');
	return pos ? pos + 1 : func;
#endif
}

int gettimeofday(struct timeval* tp, void* tzp) {
	auto now_stamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	tp->tv_sec = (decltype(tp->tv_sec))(now_stamp / 1000000LL);
	tp->tv_usec = now_stamp % 1000000LL;
	return 0;
}

string getThreadName() {
#if ((defined(__linux) || defined(__linux__)) && !defined(ANDROID)) || (defined(__MACH__) || defined(__APPLE__)) || (defined(ANDROID) && __ANDROID_API__ >= 26) || defined(__MINGW32__)
	string ret;
	ret.resize(32);
	auto tid = pthread_self();
	pthread_getname_np(tid, (char*)ret.data(), ret.size());
	if (ret[0]) {
		ret.resize(strlen(ret.data()));
		return ret;
	}
	return to_string((uint64_t)tid);
#elif defined(_MSC_VER)
	using GetThreadDescriptionFunc = HRESULT(WINAPI*)(_In_ HANDLE hThread, _In_ PWSTR* ppszThreadDescription);
	static auto getThreadDescription = reinterpret_cast<GetThreadDescriptionFunc>(::GetProcAddress(::GetModuleHandleA("Kernel32.dll"), "GetThreadDescription"));

	if (!getThreadDescription) {
		std::ostringstream ss;
		ss << std::this_thread::get_id();
		return ss.str();
	}
	else {
		PWSTR data;
		HRESULT hr = getThreadDescription(GetCurrentThread(), &data);
		if (SUCCEEDED(hr) && data[0] != '\0') {
			char threadName[MAX_PATH];
			size_t numCharsConverted;
			errno_t charResult = wcstombs_s(&numCharsConverted, threadName, data, MAX_PATH - 1);
			if (charResult == 0) {
				LocalFree(data);
				std::ostringstream ss;
				ss << threadName;
				return ss.str();
			}
			else {
				if (data) {
					LocalFree(data);
				}
				return to_string((uint64_t)GetCurrentThreadId());
			}
		}
		else {
			if (data) {
				LocalFree(data);
			}
			return to_string((uint64_t)GetCurrentThreadId());
		}
	}
#else
	if (!thread_name.empty()) {
		return thread_name;
	}
	std::ostringstream ss;
	ss << std::this_thread::get_id();
	return ss.str();
#endif
}

LogContext::LogContext(LogerLevel level, const char* file, const char* function, int line)
	: _level(level), _line(line), _file(getFileName(file)), _function(getFunctionName(function))
	 {
	gettimeofday(&_tv, nullptr);
	_thread_name = getThreadName();
}

const string& LogContext::str() {
	if (_got_content) {
		return _content;
	}
	_content = ostringstream::str();
	_got_content = true;
	return _content;
}

/*
int Log::operator()(const char* tag, const char* lpszFormat, ...)
{
	return -1;
}

int Log::operator()(const string& tag, const char* lpszFormat, ...)
{
	return -1;
}
*/

Log& Log::operator<<(ostream& (*f)(ostream&))
{
	if (!mContext) {
		return *this;
	}

	auto text = mContext->str();
	//_logger.write(mContext);
	LogV(TAG, "%s",text.c_str());
	mContext.reset();

	return *this;
}

}
}
