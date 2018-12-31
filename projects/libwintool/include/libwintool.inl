#pragma once

#ifdef _WIN_TOOL_DLL
#define WIN_CLASS	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define WIN_CLASS	__declspec(dllimport)
#else
#define WIN_CLASS
#endif
#endif

#include "core/looper/libcorelooper.inl"
