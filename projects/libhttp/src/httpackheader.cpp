#include "stdafx.h"
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
#ifndef _MINI_HTTP
		//"Server: %s\r\n"
		"Connection: %s\r\n"
#endif
		"%s"
		,
		m_httpVersion.empty() ? "1.1" : m_httpVersion.c_str(),
		m_statusCode.empty() ? "200 OK" : m_statusCode.c_str(),
		m_contentType.empty() ? "" : m_contentType.c_str(),
#ifndef _MINI_HTTP
		//m_serverDesc.c_str(),
		m_connection.IsEmpty() ? "Keep-Alive" : m_connection.c_str(),
#endif

		""//hold place
	);

	auto&  items = m_fields.GetItems();
	{
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			StringTool::AppendFormat(header,
				"%s: %s\r\n",
				iter->name.c_str(),
				iter->value.c_str()
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
