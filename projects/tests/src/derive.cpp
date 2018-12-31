#include "stdafx.h"
#include "derive.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;

Derive::~Derive()
{
	{
		//auto obj = mObject.lock();
		//DV("%s,obj=%p", __func__, obj);
	}

	{
		auto looper = mLockLooper.lock();
		if (looper)
		{
			looper->sendMessage(0);
		}
	}

}
