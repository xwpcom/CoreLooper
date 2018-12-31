#include "stdafx.h"
#include "textprotocol.h"
#include "textseparator.h"

using namespace std;

namespace Bear {
namespace Core
{
TextProtocol::TextProtocol(const char *psz, char sepChar)
{
	m_sepChar = sepChar;
	if (psz)
	{
		Parse(psz, m_sepChar);
	}
}

TextProtocol::~TextProtocol()
{
	Empty();
}

void TextProtocol::Empty()
{
	mBundle.clear();
}

//ps格式:
//name0=value0\r\n
//name1=value1\r\n
//...
int TextProtocol::Parse(const char *sz, char sepChar)
{
	Empty();

	TextSeparator demux(sz, "\r\n");
	string item;
	while (1)
	{
		int ret = demux.GetNext(item);
		if (ret)
		{
			break;
		}

		if (item.empty())
		{
			continue;
		}

		auto pos = item.find(sepChar);
		string name, value;
		if (pos == string::npos)
		{
			name = item;
		}
		else
		{
			name = item.substr(0, pos);
			value = item.substr(pos + 1);
		}

		mBundle.Set(name, value);
	}

	return 0;
}

int TextProtocol::ParseSingleLine(const string& sz, const string& sepItem, const string& sepItems)
{
	Empty();

	TextSeparator obj(sz.c_str(), sepItems.c_str());
	//string item;
	string item;
	string name, value;
	while (1)
	{
		if (obj.GetNext(item))
		{
			break;
		}

		StringTool::Trim(item, ' ');
		StringTool::Replace(item,"\"", "");
		//StringTool::Trim(item,' ');

		//DV("item=[%s]", item.c_str());

		TextSeparator obj2(item.c_str(), sepItem.c_str());
		obj2.GetNext(name);
		obj2.GetNext(value);
		//DV("[%s]=[%s]", name.c_str(), value.c_str());
		mBundle.Set(name, value);
	}


	return 0;
}


}
}