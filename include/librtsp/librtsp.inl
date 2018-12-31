#pragma once

#include "base/base.inl"
#include "libav/libav.inl"
#include "libhttp/libhttp.inl"
#include "libxml/libxml.inl"

#ifdef _LIB_RTSP_DLL
#define RTSP_CLASS	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define RTSP_CLASS	__declspec(dllimport)
#else
#define RTSP_CLASS
#endif
#endif

#include "rtsphandler.h"
#include "rtspserver.h"
#include "rtspserverhandler.h"