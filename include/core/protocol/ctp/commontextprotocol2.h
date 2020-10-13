﻿#pragma once
#include<string>
#include<map>
#include <unordered_map>

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

using namespace std;

#define CTP_CMD				"_cmd"
#define CTP_CMD_SEQ			"_seq"
#define CTP_CMD_BODY_LENGTH	"_bytes"

class CommonTextProtocol2;
class CORE_EXPORT CommonTextProtocolCB2
{
public:
	virtual ~CommonTextProtocolCB2() {}

	//收到对方的命令时调用本接口//
	virtual void OnCommand(CommonTextProtocol2 *obj, const string&cmd, const Bundle& inputBundle, const ByteBuffer& inputBody) = 0;

	//协议等出错时调用本接口
	//正常情况下不会触发,仅用于开发调试//
	virtual void OnError(CommonTextProtocol2 *obj, int error, const string& desc) = 0;

	//有数据要发给对方时，会调用本接口//
	virtual void Output(CommonTextProtocol2 *obj, const ByteBuffer& data) = 0;
};

//XiongWanPing 2017.09.23
//通用文本协议,详情见文档"CTP协议.docx"
//为方便移植，应该只采用c++和stl
//本类只负责实现解包和打包的框架
//上层需要实现CommonTextProtocolCB2来完成网络收发和具体业务处理
class CORE_EXPORT CommonTextProtocol2
{
public:
	virtual ~CommonTextProtocol2() {};
	virtual void SetCB(CommonTextProtocolCB2*) = 0;

	//收到网络包时调用本接口来解析处理
	virtual int Input(void *data, int dataBytes) = 0;

	//cmd中不要包含如下字符.=
	virtual int AddCommand(const string&cmd) = 0;

	//bundle中可传自定义参数,数据不能包括\r\n,如有必要，可用base64编码后再传入
	//bunlde字段名不能为CTP_CMD,CTP_CMD_SEQ,CTP_CMD_BODY_LENGTH冲突
	virtual int AddCommand(const string&cmd, const Bundle& bundle) = 0;

	//body中可传任意数据
	//建议单个报文不要超过16KB
	//如果要传大数据，应用层要自行设计分包机制
	virtual int AddCommand(const string&cmd, const Bundle& bundle, const ByteBuffer& body) = 0;

	virtual void ResetX() = 0;
};

class CORE_EXPORT CommonTextProtocolFactory2
{
public:
	static CommonTextProtocol2* Create();
	static void Destroy(CommonTextProtocol2*& obj);
};

}
}
}
}
}