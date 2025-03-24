#pragma once
#include <ctime>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <atomic>
#include <unordered_map>

namespace Core {

#define StrPrinter ::Core::_StrPrinter()
class _StrPrinter : public std::string {
public:
	_StrPrinter() {}

	template<typename T>
	_StrPrinter& operator <<(T&& data) {
		_stream << std::forward<T>(data);
		this->std::string::operator=(_stream.str());
		return *this;
	}

	std::string operator <<(std::ostream& (*f)(std::ostream&)) const {
		return *this;
	}

private:
	std::stringstream _stream;
};

}
