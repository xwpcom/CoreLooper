#pragma once
#include "../xmlackbase.h"
namespace Bear {
namespace Core {
namespace Xml {

class XML_CLASS Xml_RequestDownloadFile :public XmlAckBase
{
	SUPER(XmlAckBase)
public:
	Xml_RequestDownloadFile();
	~Xml_RequestDownloadFile();

	int Parse(std::string xmlAck);
	void Reset();

	DWORD mTotalBytes;
};

}
}
}
