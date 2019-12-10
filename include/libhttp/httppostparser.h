#pragma once

#include "base/bytebuffer.h"
#include "httprequesthandler.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

struct tagHttpPostField
{
	tagHttpPostField()
	{
		m_bFinished = false;
		mInbox = make_shared<ByteBuffer>();
	}

	std::string 		mName;
	std::string 		m_filename;
	std::shared_ptr<ByteBuffer>	mInbox;
	bool		m_bFinished;//数据是否接收完整
};

//XiongWanPing 2011.10.31
//解析服务器从HTTP POST接收到的数据
//用于上传配置文件等
class HTTP_EXPORT HttpPostParser
{
public:
	HttpPostParser();
	~HttpPostParser();

	vector<tagHttpPostField>	m_lstField;
#ifdef _DEBUG
	DWORD		m_dwRecvBytes;
#endif

	tagHttpPostField* GetField(const char *fieldName);

	//是否发生了错误	
	BOOL IsError()const
	{
		return (m_ePostParserStatus & ePostParserStatus_Error) ? TRUE : FALSE;
	}

	BOOL IsFinished();

	int Input(const LPBYTE pData, int cbData);
	void Reset();
#ifdef _DEBUG
	void DumpField();
	static int Test();
#endif
protected:
	void Empty();

	enum ePostParserStatus
	{
		ePostParserStatus_Idle,

		//还在等待完整的http header,用来解析Content-Type和boundary
		//Content-Type: multipart/form-data; boundary=---------------------------7db23211280430
		ePostParserStatus_WaitingHttpHeader,
		ePostParserStatus_WaitingFieldHeader,
		ePostParserStatus_ParseFieldBody,

		//所有出错码从ePostParserStatus_Error开始,最高位为1表示出错状态
		ePostParserStatus_Error = 0x80000000,
		ePostParserStatus_Error_NoContentLength,
		ePostParserStatus_Error_NoBoundary,
		ePostParserStatus_Error_InvalidFieldHeader,
	};

	void SwitchStatus(ePostParserStatus status);

	ePostParserStatus m_ePostParserStatus;
	UINT		m_ContentLength;			//http body总长度,用来判断是否过大，数据是否接收完整
	std::string 	m_boundary;

	ByteBuffer	m_inbox;

	enum eParseResult
	{
		eParseResult_Error,					//解析出错,停止解析所有数据并且拒绝接收新数据
		eParseResult_NeedMoreData,			//m_inbox中的数据不完整,需要新数据才能继续解析
		eParseResult_ContinueParse,			//还可以继续解析
		eParseResult_DataTooBig,			//上传的数据太大
	};
	eParseResult Parse();
	eParseResult ParseHelper();
};

}
}
}
}