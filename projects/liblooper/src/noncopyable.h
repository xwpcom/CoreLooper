#pragma once

class noncopyable {
protected:
	noncopyable() {}
	~noncopyable() {}
private:
	//禁止拷贝  [AUTO-TRANSLATED:e8af72e3]
	//Prohibit copying
	noncopyable(const noncopyable& that) = delete;
	noncopyable(noncopyable&& that) = delete;
	noncopyable& operator=(const noncopyable& that) = delete;
	noncopyable& operator=(noncopyable&& that) = delete;
};
