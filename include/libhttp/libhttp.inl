#pragma once

#include "../../include/core/base/base.inl"
#include "../../include/libcrypt/libcrypt.inl"

#ifdef _HTTP_DLL
#define HTTP_EXPORT	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define HTTP_EXPORT	__declspec(dllimport)
#else
#define HTTP_EXPORT
#endif
#endif

#include "ajaxcommandhandler.h"
#include "ajaxhandler.h"
#include "jsonhandler.h"
#include "httpackheader.h"
#include "httpackparser.h"
//#include "httpclient.h"
#include "httpconfig.h"
#include "httphandler.h"
#include "httptool.h"
#include "httpserver.h"
#include "httpposthandler.h"
#include "httppost.h"
#include "httpget.h"
#include "httpacker.h"
#include "postjsonmanager.h"
