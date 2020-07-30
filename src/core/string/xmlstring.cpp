#include "stdafx.h"
#include "xmlstring.h"
using namespace std;

namespace Bear {
namespace Core
{
string CreatePropStr(shared_ptr<NameValue> props)
{
	string sz;
	if (props)
	{
		auto& items = props->GetItemsX();
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			StringTool::AppendFormat(sz," %s=\"%s\"", iter->first.c_str(), iter->second.c_str());
		}
	}

	return sz;
}

void XmlString::AddItem(const char *name, const string& value, shared_ptr<NameValue> props)
{
	ASSERT(name && name[0]);
	string propStr = CreatePropStr(props);
	StringTool::AppendFormat(mString,"<%s%s>%s</%s>", name, propStr.c_str(), StringTool::xml(value).c_str(), name);
}

void XmlString::AddItem(const char *name, int value, shared_ptr<NameValue> props)
{
	ASSERT(name && name[0]);

	string propStr = CreatePropStr(props);
	StringTool::AppendFormat(mString,"<%s%s>%d</%s>", name, propStr.c_str(), value, name);
}

void XmlString::AddItem(const char *name, long long value, shared_ptr<NameValue> props)
{
	ASSERT(name && name[0]);

	string propStr = CreatePropStr(props);
	StringTool::AppendFormat(mString,"<%s%s>" FMT_LONGLONG "</%s>", name, propStr.c_str(), value, name);
}

void XmlString::AddItem(const char *name, DWORD value, shared_ptr<NameValue> props)
{
	ASSERT(name && name[0]);

	string propStr = CreatePropStr(props);
	StringTool::AppendFormat(mString,"<%s%s>%d</%s>", name, propStr.c_str(), value, name);
}

void XmlString::AddItem(const char *name, long value, shared_ptr<NameValue> props)
{
	ASSERT(name && name[0]);

	string propStr = CreatePropStr(props);
	StringTool::AppendFormat(mString,"<%s%s>%d</%s>", name, propStr.c_str(), value, name);
}
}
}
