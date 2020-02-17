#pragma once

#include "base/base.inl"

#ifdef _LIB_XML_DLL
#define XML_CLASS	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define XML_CLASS	__declspec(dllimport)
#else
#define XML_CLASS
#endif
#endif

#include "xmlackbase.h"
#include "xmlparser.h"
