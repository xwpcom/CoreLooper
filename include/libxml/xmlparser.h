#pragma once

//编译pugixml.cpp时不要用stdafx.h
#include "../depends/pugixml-1.7/src/pugixml.hpp"

namespace Bear {
namespace Core {
namespace Xml {
//XiongWanPing 2013.08.05
//采用pugixml解析xml数据

class XML_CLASS XmlParser
{
public:
	XmlParser();
	virtual ~XmlParser();

	static int Test();

	int Parse(const std::string& xml);
	int Parse(const char *text);

	int GetInt(std::string path, int defaultValue = 0);
	bool GetBool(pugi::xml_node node, std::string path, bool defaultValue = false);
	int GetInt(pugi::xml_node node, std::string path, int defaultValue = 0);
	double GetDouble(pugi::xml_node node, std::string path, double defaultValue = 0);
	long GetLong(pugi::xml_node node, std::string path, long defaultValue = 0);
	long long GetLongLong(pugi::xml_node node, std::string path, long long defaultValue = 0);

	std::string GetString(std::string path, const std::string& defaultValue = "");
	std::string GetString(pugi::xml_node node, std::string path, const std::string& defaultValue = "");

	int GetNodeCount(std::string path);
	int GetNodeCount(pugi::xml_node node, std::string path);

	pugi::xml_node FindNode(std::string path, pugi::xml_node *startNode = NULL);
	pugi::xml_node FindNode(pugi::xml_node node, std::string path);

protected:

	pugi::xml_document m_doc;
};
}
}
}
