#pragma once

#include "base/bundle.h"
namespace Bear {
namespace Core
{
//XiongWanPing 2011.10.21
//文本协议解析
class CORE_EXPORT TextProtocol
{
public:
	TextProtocol(const char *psz = NULL, char sepChar = '=');
	TextProtocol(const std::string& sz, char sepChar = '=')
	{
		m_sepChar = sepChar;
		Parse(sz.c_str(), m_sepChar);
	}
	virtual ~TextProtocol();
	int Parse(const char *psz, char sepChar = '=');
	int ParseSingleLine(const std::string& sz, const std::string& sepItem="=", const std::string& sepItems=",");

protected:
	void Empty();
	char m_sepChar;
public:
	Bundle mBundle;
};

}
}