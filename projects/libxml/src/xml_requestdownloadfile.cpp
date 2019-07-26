#include "stdafx.h"
#include "xml_requestdownloadfile.h"
#include "xmlparser.h"

using namespace std;
namespace Bear {
namespace Core {
namespace Xml {

Xml_RequestDownloadFile::Xml_RequestDownloadFile()
{
	mTotalBytes = 0;
}

Xml_RequestDownloadFile::~Xml_RequestDownloadFile()
{

}

int Xml_RequestDownloadFile::Parse(string xmlAck)
{
	Reset();

	__super::Parse(xmlAck);

	XmlParser xml;
	xml.Parse(xmlAck);

	pugi::xml_node Result = xml.FindNode("Result");
	int Error = xml.GetInt(Result, "Error");
	if (Error)
	{
		return -1;
	}

	mTotalBytes = xml.GetInt(Result, "TotalBytes");
	return 0;
}

void Xml_RequestDownloadFile::Reset()
{
	__super::Reset();

	mTotalBytes = 0;
}

}
}
}
