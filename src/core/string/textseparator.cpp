#include "stdafx.h"
#include "string/textseparator.h"
using namespace std;

namespace Bear {
namespace Core
{
TextSeparator::TextSeparator(const char *text, const char *delimiter)
{
	mBegin = text;
	mDelimiter = delimiter;

	ASSERT(mBegin);
	ASSERT(mDelimiter);
}

TextSeparator::TextSeparator(const string& text, const char* delimiter)
{
	mBegin = text.c_str();
	mDelimiter = delimiter;

	ASSERT(mBegin);
	ASSERT(mDelimiter);
}

int TextSeparator::GetNext(string& sz, eTextFlag flag)
{
	sz.clear();

	if (!mBegin)
	{
		return -1;
	}

	const char *psz = strstr(mBegin, mDelimiter);
	if (!psz)
	{
		int ret = (mBegin[0] == 0) ? -1 : 0;
		sz = mBegin;
		mBegin = nullptr;
		return ret;
	}

	string str(mBegin, (int)(psz - mBegin));
	sz = str;
	mBegin = psz + strlen(mDelimiter);

	if (flag & eTextFlag_TrimTail)
	{
		StringTool::Trim(sz, '\r');
	}
	return 0;
}

void TextSeparator::Parse(const string& text, const string& delimiter, vector<string>& items)
{
	TextSeparator obj(text.c_str(), delimiter.c_str());
	items.clear();

	string item;
	while (1)
	{
		if (obj.GetNext(item))
		{
			break;
		}

		items.push_back(item);
	}

}

}
}