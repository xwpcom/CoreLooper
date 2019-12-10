#pragma once

#ifdef _DB_EXPORT
#define DB_EXPORT __declspec(dllexport)
#else
#ifdef _MSC_VER
#define DB_EXPORT __declspec(dllimport)
#else
#define DB_EXPORT
#endif
#endif


namespace Database
{
using namespace Bear::Core;
}


#include "mysqlex.h"
#include "mysqlres.h"

