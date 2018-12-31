#include "stdafx.h"
#include "xmlackbase.h"
#include "xmlparser.h"
#include "net/urltool.h"

using namespace std;
using namespace Bear::Core::Net;
namespace Bear {
namespace Core {
namespace Xml {

XmlAckBase::XmlAckBase()
{
	Error = -1;
}

XmlAckBase::~XmlAckBase()
{
}

//一般用来解析setXXX.xml
//getXXX.xml的由子类自己解析
int XmlAckBase::Parse(string xmlAck)
{
	Reset();

	if (xmlAck.empty())
	{
		return -1;
	}

	XmlParser xml;
	xml.Parse(xmlAck);

	int ret = Parse(&xml, xmlAck);
	return ret;
}

int XmlAckBase::Parse(XmlParser* pXml, string xmlAck)
{
	XmlParser& xml = *pXml;
	pugi::xml_node Result = xml.FindNode("Result");
	if (Result)
	{
		Error = xml.GetInt(Result, "Error");
		ErrorName = xml.GetString(Result, "ErrorName");
	}

	return 0;
}

string XmlAckBase::GetUriTitle(string url)
{
	return UrlTool::GetUriTitle(url);
}

string XmlAckBase::GetUri(string url)
{
	auto pos = url.find('.');
	if (pos != -1)
	{
		auto end = pos + 4;////包括后面的.xml

		int start = 0;
		char ch = url[0];
		if (ch == '/' || ch == '\\')
		{
			start = 1;
		}

		return url.substr(start, end - start);
	}

	return "";
}

}
}
}