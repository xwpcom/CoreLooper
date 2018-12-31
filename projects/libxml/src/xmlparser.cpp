#include "stdafx.h"
#include "XmlParser.h"

using namespace std;
using namespace pugi;

namespace Bear {
namespace Core {
namespace Xml {

XmlParser::XmlParser()
{
}

XmlParser::~XmlParser()
{
}

int XmlParser::Parse(const string& xml)
{
	if (xml.empty() || xml[0] == 0)
	{
		return -1;
	}

	pugi::xml_parse_result result = m_doc.load(xml.c_str());
	if (result.status != status_ok)
	{
		DW("fail load xml");
		return -1;
	}

	return 0;
}

pugi::xml_node XmlParser::FindNode(pugi::xml_node node, string path)
{
	return FindNode(path, &node);
}

//支持下标和多级
pugi::xml_node XmlParser::FindNode(string path, pugi::xml_node *startNode)
{
	//ASSERT(startNode == NULL);
	xml_node node;
	if (startNode)
	{
		node = *startNode;
	}

	vector<string> items;
	TextSeparator::Parse(path, ".", items);
	int i = 0;
	for (auto iter=items.begin();iter!=items.end();++iter,++i)
	{
		auto& name = *iter;

		UINT idx = 0;
		auto pos = name.find('[');//支持下标
		if (pos != -1)
		{
			idx = atoi(name.c_str() + pos + 1);
			name = name.substr(0,pos);
		}

		if (i > 0)
		{
			node = node.child(name.c_str());
		}
		else
		{
			if (node.empty())
			{
				node = m_doc.child(name.c_str());
			}
			else
			{
				node = node.child(name.c_str());
			}
		}

		if (node.empty())
		{
			break;
		}

		bool bFind = false;
		UINT idxThis = 0;
		while (!node.empty())
		{
			if (idxThis == idx)
			{
				bFind = true;
				break;
			}

			node = node.next_sibling(name.c_str());
			idxThis++;
		}

		if (!bFind)
		{
			node = pugi::xml_node();
			break;
		}
	}

	return node;
}

int XmlParser::GetInt(string path, int defaultValue)
{
	string def=StringTool::Format("%d", defaultValue);

	string value = GetString(path, def);
	if (value.empty())
	{
		return defaultValue;
	}

	return atoi(value.c_str());
}

bool XmlParser::GetBool(pugi::xml_node node, string path, bool defaultValue)
{
	return !!GetInt(node, path, defaultValue);
}

int XmlParser::GetInt(pugi::xml_node node, string path, int defaultValue)
{
	string sz=StringTool::Format("%d", defaultValue);
	sz = GetString(node, path, sz);
	return atoi(sz.c_str());
}

long XmlParser::GetLong(pugi::xml_node node, string path, long defaultValue)
{
	string sz=StringTool::Format("%d", defaultValue);
	sz = GetString(node, path, sz);
	return atol(sz.c_str());
}

long long XmlParser::GetLongLong(pugi::xml_node node, string path, long long defaultValue)
{
	string sz=StringTool::Format("%d", defaultValue);
	sz = GetString(node, path, sz);
	return atoll(sz.c_str());
}

double XmlParser::GetDouble(pugi::xml_node node, string path, double defaultValue)
{
	auto sz=StringTool::Format("%f", defaultValue);
	sz = GetString(node, path, sz);

	char *psz = nullptr;
	return strtod(sz.c_str(), &psz);
}

string XmlParser::GetString(pugi::xml_node node, string path, const string& defaultValue)
{
	int pos = path.rfind('.');

	if (pos == -1)
	{
		pos = path.length();
	}

	string ret = defaultValue;

	if (pos == path.length())
	{
		node = node.child(path.c_str());
		return node.text().as_string();
	}

	string parentPath = path.substr(0,pos);
	node = FindNode(parentPath, &node);
	if (node.empty())
	{
		return ret;
	}


	const char *lastItemName = path.c_str() + pos + 1;//最后一个item可能是attribute或者child node

	pugi::xml_node child = node.child(lastItemName);
	if (!child.empty())
	{
		return child.text().as_string();
	}

	pugi::xml_attribute attr = node.attribute(lastItemName);
	if (!attr.empty())
	{
		return attr.value();
	}

	return ret;
}

string XmlParser::GetString(string path, const string& defaultValue)
{
	vector<string> items;
	TextSeparator::Parse(path.c_str(),".",items);
	
	string ret = defaultValue;
	xml_node node;

	int i = 0;
	int nc = (int)items.size();
	for (auto iter=items.begin();iter!=items.end();++iter,++i)
	{
		auto& item = *iter;;
		if (i > 0)
		{
			node = node.child(item.c_str());
		}
		else
		{
			node = m_doc.child(item.c_str());
		}

		if (node.empty())
		{
			break;
		}

		if (i == nc - 1)
		{
			ret = node.text().as_string(defaultValue.c_str());
		}
	}

	return ret;
}

int XmlParser::GetNodeCount(pugi::xml_node node, string name)
{
	int nc = 0;

	node = node.child(name.c_str());
	while (!node.empty())
	{
		nc++;
		node = node.next_sibling(name.c_str());
	}

	return nc;
}

int XmlParser::GetNodeCount(string path)
{
	pugi::xml_node node = FindNode(path);

	int nc = 0;
	for (; !node.empty(); node = node.next_sibling())
	{
		nc++;
	}

	return nc;
}

int XmlParser::Test()
{
	return 0;
}

}
}
}