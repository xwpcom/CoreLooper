#pragma once

#include "xml/xmlerror.h"

namespace Bear {
namespace Core {
namespace Xml {

class XmlParser;
//XiongWanPing 2013.08.06
//ajax ack xml框架
class XML_CLASS XmlAckBase
{
public:
	XmlAckBase();
	virtual ~XmlAckBase();

	virtual int Parse(std::string xmlAck);
	virtual int Parse(XmlParser* xml, std::string xmlAck);
	virtual void Reset()
	{
		Error = -1;
	}
	bool Success()const
	{
		return Error == 0;
	}

	static std::string GetUri(std::string url);
	static std::string GetUriTitle(std::string url);
public:
	int			Error;
	std::string	ErrorName;
};

}
}
}
