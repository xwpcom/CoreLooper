#include "stdafx.h"
#include "log.h"
#include "looper/looper.h"

using namespace Bear::Core;

#ifdef _CONFIG_HANDLER_LOG

void Log::v(const char *tag, const char*pszFormat, ...)
{
	unused(tag);

	LOG_IMPL(tag,eLogV);
}

void Log::d(const char *tag, const char*pszFormat, ...)
{
	unused(tag);

	LOG_IMPL(tag, eLogD);
}

void Log::i(const char *tag, const char*pszFormat, ...)
{
	unused(tag);

	LOG_IMPL(tag, eLogI);
}

void Log::w(const char *tag, const char*pszFormat, ...)
{
	UNUSED(tag);

	LOG_IMPL(tag, eLogW);
}

void Log::e(const char *tag, const char*pszFormat, ...)
{
	UNUSED(tag);

	LOG_IMPL(tag, eLogE);
}

#endif
