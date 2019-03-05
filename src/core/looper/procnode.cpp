#include "stdafx.h"
#include "procnode.h"
#include "handler.h"

using namespace std;

namespace Bear {
namespace Core
{
/*
可以仿照linux /proc那样，用Handler机制搞一个tree node
可展示app当前的运行状态数据，可修改
可以枚举所有的node,每个node有get/set和desc等接口
做一个网页,提供editBox和set
可修改指定path的数据
一个很好的用途是修改指定模块的log级别
/app/rtsp/loglevel     xxx

Proc框架采用weekptr来引用,由于各Handler可能处于不同的looper中，所以Proc框架采用sendMessage来get/set等收集和修改
这是一个很有用的功能
节点/子节点/数据
list
*/

ProcNode::ProcNode()
{
}

ProcNode::~ProcNode()
{

}

#define IMPLEMENT_TYPE(type,Type,field)	\
int ProcNode::Bind(string name, type & value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter) \
{																																				  \
	return field.Bind(name, value, desc, flags, getter, setter);																				  \
}																																				  \
																																				  \
type ProcNode::Get##Type(string name)																										  \
{																																				  \
	return field.Get(name);																														  \
}																																				  \
																																				  \
int ProcNode::Set##Type(string name, type value)																								  \
{																																				  \
	return field.Set(name,value);																												  \
}

IMPLEMENT_TYPE(bool, Bool, mBools)
IMPLEMENT_TYPE(BYTE, Byte, mBytes)
IMPLEMENT_TYPE(int, Int, mInts)
IMPLEMENT_TYPE(WORD, Word, mWords)
IMPLEMENT_TYPE(DWORD, Dword, mDwords)
IMPLEMENT_TYPE(double, Double, mDoubles)
IMPLEMENT_TYPE(LONGLONG, LongLong, mLongLongs)
IMPLEMENT_TYPE(ULONGLONG, ULongLong, mULongLongs)
IMPLEMENT_TYPE(string, String, mStrings)

template<>string ProcNode::tagItems<bool>::mItemFormat = "%d";
template<>string ProcNode::tagItems<BYTE>::mItemFormat = "%d";
template<>string ProcNode::tagItems<int>::mItemFormat = "%d";
template<>string ProcNode::tagItems<WORD>::mItemFormat = "%d";
template<>string ProcNode::tagItems<DWORD>::mItemFormat = "%d";
template<>string ProcNode::tagItems<double>::mItemFormat = "%f";
template<>string ProcNode::tagItems<LONGLONG>::mItemFormat = FMT_LONGLONG;
template<>string ProcNode::tagItems<ULONGLONG>::mItemFormat = FMT_LONGLONG;
template<>string ProcNode::tagItems<string>::mItemFormat = "%s";

void ProcNode::DumpData(string& xml)
{
	mBools.Dump(xml);
	mBytes.Dump(xml);
	mInts.Dump(xml);
	mWords.Dump(xml);
	mDwords.Dump(xml);
	mDoubles.Dump(xml);
	mLongLongs.Dump(xml);
	mULongLongs.Dump(xml);
	mStrings.DumpString(xml);
}
}
}
