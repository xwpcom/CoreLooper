#include "stdafx.h"
#include "httppostparser.h"
using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpPostParser::HttpPostParser()
{
	Reset();
}

HttpPostParser::~HttpPostParser()
{
	Empty();
}

void HttpPostParser::Reset()
{
	m_ePostParserStatus = ePostParserStatus_Idle;
	m_ContentLength = 0;

#ifdef _DEBUG
	m_dwRecvBytes = 0;
#endif

}

void HttpPostParser::Empty()
{
	m_lstField.clear();
}


void HttpPostParser::SwitchStatus(ePostParserStatus status)
{
	m_ePostParserStatus = status;
}


//输入服务器接收到的HTTP POST数据,可分批次输入,不需要一次性输入完整
//pData指向的数据不需要以'\0'结尾
int HttpPostParser::Input(const LPBYTE pData, int cbData)
{
	if (!pData || cbData <= 0)
	{
		ASSERT(FALSE);
		return 0;
	}
#ifdef _DEBUG
	m_dwRecvBytes += cbData;
#endif

	if (!m_inbox.IsInited())
	{
		m_inbox.SetBufferSize(2 * 1024, 8 * 1024);
	}

	LPBYTE psrc = pData;
	int cbsrc = cbData;
	while (cbsrc > 0)
	{
		if (m_ePostParserStatus & ePostParserStatus_Error)
		{
			//已出错
			return -1;
		}

		int ret = 0;
		m_inbox.MoveToHead();
		int maxWrite = m_inbox.GetTailFreeSize() - 1;//-1是为'\0'保留空间
		ret = m_inbox.Write(psrc, MIN(cbsrc, maxWrite));
		if (ret <= 0)
		{
			DW("inbox full");
			ASSERT(FALSE);
			SwitchStatus(ePostParserStatus_Error);
			return -1;
		}

		psrc += ret;
		cbsrc -= ret;

		eParseResult res = Parse();
		if (res == eParseResult_Error || IsError())
		{
			return -1;
		}
	}

	return 0;
}

//解析m_inbox中的数据
HttpPostParser::eParseResult HttpPostParser::Parse()
{
	m_inbox.MakeSureEndWithNull();//确保字符串结尾

	eParseResult res = eParseResult_Error;
	int bytes = m_inbox.GetActualDataLength();
	while ((res = ParseHelper()) == eParseResult_ContinueParse)
	{
		//确保不要进入死循环
		int new_bytes = m_inbox.GetActualDataLength();
		if (bytes == new_bytes)
		{
			DW("detect dead loop");
			ASSERT(FALSE);
			break;
		}

		bytes = new_bytes;
		if (new_bytes == 0)
		{
			break;
		}
	}

	return res;
}

HttpPostParser::eParseResult HttpPostParser::ParseHelper()
{
	int ret = 0;
	eParseResult res = eParseResult_ContinueParse;

	const char *ps = (char *)m_inbox.GetDataPointer();
	int ps_len = m_inbox.GetActualDataLength();
	//下面解析ps中的数据

	switch (m_ePostParserStatus)
	{
	case ePostParserStatus_Idle:
	case ePostParserStatus_WaitingHttpHeader:
	{
		ASSERT(m_ContentLength == 0 && m_boundary.empty());

		int httpHeaderLen = 0;
		char *pTail = NULL;
		char chSave;
		{
			//检查是否接收到完整的http header
			const char *key = "\r\n\r\n";
			const char *p = strstr(ps, key);
			if (!p)
			{
				res = eParseResult_NeedMoreData;
				break;
			}

			p += strlen(key);
			pTail = (char*)p;
			chSave = *pTail;
			*pTail = 0;
			httpHeaderLen = (int)(p - ps);
		}

		//已收到完整http header,解析里面的Content-Length和boundary
		{
			const char *key = "Content-Length: ";
			const char *p = strstr(ps, key);
			if (!p)
			{
				SwitchStatus(ePostParserStatus_Error_NoContentLength);
				return eParseResult_Error;
			}
			p += strlen(key);
			m_ContentLength = atoi(p);

			if (m_ContentLength >= 64 * 1024)
			{
				return eParseResult_DataTooBig;
			}
		}

		{
			const char *key = "boundary=";
			const char *p = strstr(ps, key);
			if (p)
			{
				p += strlen(key);
				const char *pend = strchr(p, '\r');
				if (pend)
				{
					m_boundary = string(p, pend - p);
				}
			}
		}

		if (m_ContentLength == 0)
		{
			SwitchStatus(ePostParserStatus_Error_NoContentLength);
			return eParseResult_Error;
		}

		if (m_boundary.empty())
		{
			SwitchStatus(ePostParserStatus_Error_NoBoundary);
			return eParseResult_Error;
		}

		*pTail = chSave;

		//http header解析成功,要转到http body
		m_inbox.Eat(httpHeaderLen);
		SwitchStatus(ePostParserStatus_WaitingFieldHeader);
		break;
	}
	case ePostParserStatus_WaitingFieldHeader:
	{
		int fieldHeaderLen = 0;
		//判断field header格式是否有效:
		//必须以--m_boundary开头
		//以"\r\n\r\n"结尾表示field header接收完整
		char *pTail = NULL;
		char chSave;
		{
			//检查是否接收到完整的field header
			const char *key = "\r\n\r\n";
			const char *p = strstr(ps, key);
			if (!p)
			{
				res = eParseResult_NeedMoreData;
				break;
			}

			p += strlen(key);
			pTail = (char*)p;
			chSave = *pTail;
			*pTail = 0;
			fieldHeaderLen = (int)(p - ps);
		}

		{
			//field header已接收完整,检查是否以--m_boundary开头
			if (ps[0] != '-' || ps[1] != '-')
			{
				SwitchStatus(ePostParserStatus_Error_InvalidFieldHeader);
				return eParseResult_Error;
			}
			ps += 2;

			ret = strncmp(ps, m_boundary.c_str(), m_boundary.length());
			if (ret)
			{
				SwitchStatus(ePostParserStatus_Error_InvalidFieldHeader);
				return eParseResult_Error;
			}
		}

		//下面解析出需要的字段
		{
			string  fieldName, fieldFileName;//fieldName和fieldFileName可能都为空

			//todo:name和filename本身不能包含"字符
			{
				const char *key = "name=\"";
				const char *pname = strstr(ps, key);
				if (pname)
				{
					pname += strlen(key);
					const char *pend = strchr(pname, '"');
					if (pend)
					{
						fieldName = string(pname, pend - pname);
					}
				}
			}
			{
				const char *key = "filename=\"";
				const char *pname = strstr(ps, key);
				if (pname)
				{
					pname += strlen(key);
					const char *pend = strchr(pname, '"');
					if (pend)
					{
						fieldFileName = string(pname, pend - pname);
					}
				}
			}

			tagHttpPostField item;
			item.mName = fieldName;
			item.m_filename = fieldFileName;
			m_lstField.push_back(item);

			DT("new field,name=[%s],filename=[%s]", fieldName.c_str(), fieldFileName.c_str());

			m_inbox.Eat(fieldHeaderLen);
			SwitchStatus(ePostParserStatus_ParseFieldBody);
		}

		*pTail = chSave;
		break;
	}
	case ePostParserStatus_ParseFieldBody:
	{
		//把数据写到m_lstField.tail中的inbox
		//直到检测到\r\n--m_boundary才结束
		auto nc = m_lstField.size();
		if (nc <= 0)
		{
			ASSERT(FALSE);
			SwitchStatus(ePostParserStatus_Error);
			return eParseResult_Error;
		}

		auto& field = m_lstField.back();

		ps = (char *)m_inbox.GetDataPointer();
		ps_len = m_inbox.GetActualDataLength();//-1;//由于这里处理的是hex数据，所以要屏蔽上面添加的'\0'字节
		int cbLeft = ps_len;
		int eat = 0;
		const int boundary_len = (int)m_boundary.length();
		while (cbLeft > 0)
		{
			//检测\r\n--m_boundary结束符
			BOOL bReachFieldEnd = FALSE;
			const char *pend = (const char *)memchr(ps, '\r', cbLeft);
			if (pend)
			{
				if (cbLeft <= 4 + boundary_len)
				{
					res = eParseResult_NeedMoreData;
					break;
				}

				/*
				{
					static int idx=0;
					if(idx>=145)
					{
						int x=0;
					}
					DT("pend idx=%d",idx++);

				}
				//*/

				if (pend[1] == '\n' && pend[2] == '-' && pend[3] == '-')
				{
					ret = strncmp(pend + 4, m_boundary.c_str(), boundary_len);
					if (ret == 0)
					{
						//检测到field body end
						bReachFieldEnd = TRUE;
					}
				}

				int len = (int)(pend - ps);
				if (!bReachFieldEnd)
				{
					len++;
				}

				field.mInbox->Write((LPVOID)ps, len);
				eat += len;
				cbLeft -= len;
				ps += len;

				if (bReachFieldEnd)
				{
					field.m_bFinished = TRUE;

					DT("finish field,name=[%s]", field.mName.c_str());

#if defined _DEBUG && defined _MSC_VER
					DumpField();
#endif
					SwitchStatus(ePostParserStatus_WaitingFieldHeader);
					break;
				}
			}
			else
			{
				//全是field data
				field.mInbox->Write((LPVOID)ps, cbLeft);
				eat += cbLeft;
				ps += cbLeft;
				cbLeft = 0;
				break;
			}
		}

		m_inbox.Eat(eat);
		break;
	}
	default:
	{
		ASSERT(FALSE);
		SwitchStatus(ePostParserStatus_Error);
		return eParseResult_Error;
	}
	}

	return res;
}

//是否接收完HTTP POST数据
BOOL HttpPostParser::IsFinished()
{
	return FALSE;//todo
}

tagHttpPostField* HttpPostParser::GetField(const char *fieldName)
{
	for (auto iter = m_lstField.begin(); iter != m_lstField.end(); ++iter)
	{
		if (StringTool::CompareNoCase(iter->mName, fieldName) == 0)
		{
			return &(*iter);
		}
	}

	return nullptr;
}


#ifdef _DEBUG
void HttpPostParser::DumpField()
{
	{
		for (auto iter = m_lstField.begin(); iter != m_lstField.end(); ++iter)
		{
			if (!iter->m_filename.empty())
			{
				string  file = StringTool::Format("g:\\test\\post\\field_%s.bin", iter->mName.c_str());
				File::Dump(iter->mInbox->GetDataPointer(), iter->mInbox->GetActualDataLength(), file.c_str());
			}
		}
	}

}

int HttpPostParser::Test()
{
	return 0;
}
#endif

}
}
}
}
