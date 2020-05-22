#include "stdafx.h"
#include "base.h"
#include "CppUnitTest.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;

Base::Base()
{

}
Base::~Base()
{
	auto obj = mObject.lock();
	DV("%s,obj=%p", __func__, obj);
}

