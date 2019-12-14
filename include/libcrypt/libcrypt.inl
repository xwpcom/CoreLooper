#pragma once

#include "core/base/base.inl"

#define _SUPPORT_SSL

#ifdef _CRYPT_DLL
#define CRYPT_EXT_CLASS	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define CRYPT_EXT_CLASS	__declspec(dllimport)
#else
#define CRYPT_EXT_CLASS
#endif
#endif

#include "base64ex.h"
#include "md5ex.h"
