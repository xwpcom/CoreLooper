﻿#include "stdafx.h"
#include "httpackheader.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

string  HttpAckHeader::m_serverDesc = "";

HttpAckHeader::HttpAckHeader()
{
}

HttpAckHeader::~HttpAckHeader()
{
	//测试中文
}

string  HttpAckHeader::ack(BOOL addTail)
{
	ASSERT(!m_contentType.empty());

	string  header = StringTool::Format(
		"HTTP/%s %s\r\n"
		"Content-Type: %s\r\n"
		//"Server: %s\r\n"
		"Connection: %s\r\n"
		"%s"
		,
		m_httpVersion.empty() ? "1.1" : m_httpVersion.c_str(),
		m_statusCode.empty() ? "200 OK" : m_statusCode.c_str(),
		m_contentType.empty() ? "" : m_contentType.c_str(),
		//m_serverDesc.c_str(),
		m_connection.empty() ? "Keep-Alive" : m_connection.c_str(),

		""//hold place
	);

	auto&  items = m_fields.GetItemsX();
	{
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			StringTool::AppendFormat(header,
				"%s: %s\r\n",
				iter->first.c_str(),
				iter->second.c_str()
			);
		}
	}

	if (addTail)
	{
		header += "\r\n";
	}

	return header;
}
}
}
}
}
