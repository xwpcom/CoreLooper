#pragma once
#include "core/protocol/ctp/commontextprotocol2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

//XiongWanPing 2017.09.23
//通用文本协议,详情见文档"CTP协议.docx"
//为方便移植，应该只采用c++和stl
//本类只负责实现解包和打包的框架
//上层需要实现CommonTextProtocolCB2来完成网络收发和具体业务处理
class CommonTextProtocol_Impl2 :public CommonTextProtocol2
{
public:
	CommonTextProtocol_Impl2();
	virtual ~CommonTextProtocol_Impl2();

	void SetCB(CommonTextProtocolCB2* cb)
	{
		mCB = cb;
	}
	void ResetX();

	//收到网络包时调用本接口来解析处理
	virtual int Input(void *data, int dataBytes);

protected:
	virtual int AddCommand(const string&cmd);
	virtual int AddCommand(const string&cmd, const Bundle& bundle);
	virtual int AddCommand(const string&cmd, const Bundle& bundle, const ByteBuffer& body);

	static const char *stristr(const char *psz0, const char *psz1);

	int ParseInbox();
	void ParseHeaderItems(const string& header, Bundle& headerItems);
	void OnCommand(const string&cmd, const Bundle& inputBundle, const ByteBuffer& inputBody);

	int OnError(string error);

	CommonTextProtocolCB2* mCB = nullptr;

	ByteBuffer mInputBody;
	bool mReset = false;
	int mSeq = -1;

private:
	ByteBuffer mInbox;
	ByteBuffer mOutbox;

};

}
}
}
}
}
