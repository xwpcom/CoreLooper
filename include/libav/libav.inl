#pragma once

#include "base/base.inl"
#include "libcrypt/libcrypt.inl"

#ifdef _LIB_AV_DLL
#define AV_EXPORT	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define AV_EXPORT	__declspec(dllimport)
#else
#define AV_EXPORT
#endif
#endif


#include "audio/audiorender.h"
#include "audio/audiosource.h"
#include "mediarecorder.h"
