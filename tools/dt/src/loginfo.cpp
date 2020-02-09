#include "pch.h"
#include "loginfo.h"

shared_ptr<LogItem> LogParser::Input(const LPBYTE data, int bytes)
{
	/*
	corelooper中dt.cpp
	//static length fields
	BYTE version = 1;
	box.WriteByte(version);
	box.WriteByte((BYTE)m_nLevel);
	box.Write(&pid, sizeof(pid));
	box.Write(&tid, sizeof(tid));
	box.Write(&m_nLine, sizeof(m_nLine));
	box.Write(&date, sizeof(date));
	box.Write(&time, sizeof(time));

	//TLV fields
	//T:1 bytes
	//L:4 bytes
	*/

#pragma pack(push,1)
	struct tagStaticHeader
	{
		BYTE	version;
		BYTE	level;
		DWORD	pid;
		DWORD	tid;
		int		line;
		DWORD	date;//yymmdd
		DWORD	time;//hhmmssMMM,精确到毫秒
	};
#pragma pack(pop)

	const int minBytes = sizeof(tagStaticHeader);
	if (bytes < minBytes)
	{
		ASSERT(FALSE);
		return nullptr;
	}


	enum eType
	{
		//有些项目是固定长度，所以不需要用TLV表示
		//eLevel//固定1 bytes
		//ePid,	//固定4 bytes
		//eTid,	//固定4 bytes
		//eLine,//固定4 bytes
		eAppName = 1,
		eTag = 2,
		eMsg = 3,
		eFile = 4,
	};

	auto header = (tagStaticHeader*)data;
	if (header->version != 1)
	{
		//unmatch version
		ASSERT(FALSE);
		return nullptr;
	}

	auto item = make_shared<LogItem>();
	item->level = (eDTLevel)header->level;
	item->pid = header->pid;
	item->tid = header->tid;
	item->line = header->line;
	item->date = header->date;
	item->time = header->time;

	++header;
	LPBYTE tlv = (LPBYTE)header;
	const auto tail = data + bytes;
	while (tlv < tail)
	{
		BYTE type = tlv[0];
		int length = *((int*)(tlv+1));
		if (tlv + 1+ length >= tail)
		{
			DW("invalid tlv");
			ASSERT(FALSE);
			break;
		}

		auto data = (char*)tlv + 5;

		switch (type)
		{
		case eAppName:item->appName = string(data,length); break;
		case eTag:item->tag = string(data, length); break;
		case eMsg:item->msg = string(data, length); break;
		case eFile:item->file = string(data, length); break;
		default:
		{
			ASSERT(FALSE);
			break;
		}
		}

		tlv += 1 + 4 + length;
	}

	return item;
}
