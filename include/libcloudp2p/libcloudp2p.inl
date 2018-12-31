#pragma once

#include "base/base.inl"

#ifdef _CLOUD_P2P_DLL
#define CLOUD_P2P_EXT_CLASS	__declspec(dllexport)
#else
#ifdef _MSC_VER
#define CLOUD_P2P_EXT_CLASS	__declspec(dllimport)
#else
#define CLOUD_P2P_EXT_CLASS
#endif
#endif

#include "cloudp2papi.h"
#include "cloudp2plistener.h"
#include "cloudp2pmanager.h"
#include "cloudp2pdatachannel.h"


