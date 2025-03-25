#pragma once
#include "timems.h"

namespace Core {

int64_t tickCount();
int currentPid();
int currentTid();
//tagTimeMs currentTimeMs();
int lastError();
void setThreadName(const string& name, int dwThreadID=-1);

string appPathName();
string appFolder();

}