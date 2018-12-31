#pragma once
namespace Bear {
namespace Core
{
class CORE_EXPORT Log
{
public:
	static void v(const char *tag, const char*format, ...);
	static void d(const char *tag, const char*format, ...);
	static void i(const char *tag, const char*format, ...);
	static void w(const char *tag, const char*format, ...);
	static void e(const char *tag, const char*format, ...);
};
}
}